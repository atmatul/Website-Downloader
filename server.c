#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <fcntl.h>
#include <mysql.h>

#include "lib/includes.h"
#include "lib/database.h"
#include "lib/extractor.h"

#define MAX_REQUEST_SIZE 10000
#define MAX_RESPONSE_SIZE 999999
#define NUMBER_OF_CONNECTIONS 10
#define LENGTH_OF_PATH 1000

int add_client_socket_id(int id);

int remove_client_socket_id(int id);

int process_request(char *req, char *resp);

long prepare_response_html(char *path, char **file_content);

void write_log(char *req, char *resp);

int prepare_response(char *path, char *resp);

char *root_path = "/Users/kunal/NetworkLab/public_html";

int client_socket_ids[NUMBER_OF_CONNECTIONS];

int main(int argc, char *argv[]) {
    int socket_id, req_len, pid, reuse_id;
    int is_accepting = 1;
    socklen_t len;
    char request[MAX_REQUEST_SIZE], response[MAX_RESPONSE_SIZE];
    char client_addr_ascii[50], client_addr_dns[50];
    struct sockaddr_in6 client_addr;
    struct stat root_path_stat;
    struct addrinfo hints, *server_info;
    fd_set observers;
    struct timeval timeout;

    // getting the root path from terminal
    if (argc > 0)
        if (stat(argv[1], &root_path_stat) == 0 && S_ISDIR(root_path_stat.st_mode))
            root_path = argv[1];
    if (root_path[strlen(root_path) - 1] == '/') root_path[strlen(root_path) - 1] = '\0';

    // retrieving the server address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", "1500", &hints, &server_info) != 0) {
        notify_error("Couldn't parse the address");
    }

    // preparing the socket
    socket_id = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_id < 0) notify_error("Socket creation unsuccessful.");

    setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &reuse_id,
               sizeof(reuse_id));
//  set_nonblocking(socket_id);

    bind(socket_id, server_info->ai_addr, server_info->ai_addrlen);

    for (int i = 0; i < NUMBER_OF_CONNECTIONS; i++)
        client_socket_ids[i] = -1;

    listen(socket_id, NUMBER_OF_CONNECTIONS);
    while (1) {
        // initialize "select" observers and set timeout
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int stdin_id = fileno(stdin);
        FD_ZERO(&observers);
        FD_SET(stdin_id, &observers);
        FD_SET(socket_id, &observers);
        int max_id = (socket_id > stdin_id ? socket_id : stdin_id);
        for (int i = 0; i < NUMBER_OF_CONNECTIONS; i++) {
            if (client_socket_ids[i] != -1) {
                FD_SET(client_socket_ids[i], &observers);
                if (max_id < client_socket_ids[i])
                    max_id = client_socket_ids[i];
            }
        }

        int ready = select(max_id + 1, &observers, (fd_set *) 0, (fd_set *) 0, &timeout);

        if (ready < 0) {
            notify_error("Malfunction in \"select\"");
        } else if (ready == 0) {
            continue;
        } else {
            // checking stdin for any inputs
            if (FD_ISSET(stdin_id, &observers)) {
                int command;
                command = getchar();
                if (command == 'q') {
                    printf("\nQuitting Server...\n");
                    break;
                } else if (command == 's') {
                    if (is_accepting) {
                        is_accepting = 0;
                        printf("\nHibernating Server...\n");
                    } else {
                        is_accepting = 1;
                        printf("\nWaking Server Up...\n");
                    }
                }
            }

            if (is_accepting && FD_ISSET(socket_id, &observers)) {
                int client_socket_id;
                // accept connection from client
                len = sizeof client_addr;
                client_socket_id = accept(socket_id, (struct sockaddr *) &client_addr, &len);
//        set_nonblocking(client_socket_id);
                if (client_socket_id < 0) notify_error("Accepting connection failed.");
                else add_client_socket_id(client_socket_id);
            }

            for (int i = 0; i < NUMBER_OF_CONNECTIONS; i++) {
                if ((client_socket_ids[i] >= 0) && FD_ISSET(client_socket_ids[i], &observers)) {
                    // extract client DNS, IP:PORT
                    inet_ntop(AF_INET6, &(client_addr.sin6_addr), client_addr_ascii, INET_ADDRSTRLEN);
                    getnameinfo((struct sockaddr *) &client_addr, sizeof(client_addr),
                                client_addr_dns, sizeof(client_addr_dns),
                                NULL, 0, NI_NAMEREQD);
                    // read client request
                    bzero(request, MAX_REQUEST_SIZE);
                    req_len = read(client_socket_ids[i], request, MAX_REQUEST_SIZE);
                    if (req_len >= 0) {
                        printf("Request from client (%s)[%s:%d]: (SIZE = %dB)\n",
                               client_addr_dns, client_addr_ascii,
                               ntohs(client_addr.sin6_port), req_len);
                        // process the request, extract relevant data
                        int res_len = process_request(request, response);
                        if (strlen(response) > 0) {
                            write(client_socket_ids[i], response, res_len);
                            write_log(request, response);
                        } else {
                            notify_error("Error occurred perparing response.");
                        }
                        // closing connection to client
                        close(client_socket_ids[i]);
                        FD_CLR(client_socket_ids[i], &observers);
                        remove_client_socket_id(client_socket_ids[i]);
                    }
                }
            }
        }
    }
    // closing server socket
    close(socket_id);
    freeaddrinfo(server_info);
    return 0;
}

/**
 * add client socket id to `client_socket_ids`
 * @param id - id of the client server
 * @return success code
 */
int add_client_socket_id(int id) {
    int success = 0;
    for (int i = 0; i < NUMBER_OF_CONNECTIONS; i++) {
        if (client_socket_ids[i] == -1) {
            client_socket_ids[i] = id;
            success = 1;
            break;
        }
    }
    return success;
}

/**
 * add client socket id to `client_socket_ids`
 * @param id - id of the client server
 * @return success code
 */
int remove_client_socket_id(int id) {
    int success = 0;
    for (int i = 0; i < NUMBER_OF_CONNECTIONS; i++) {
        if (client_socket_ids[i] == id) {
            client_socket_ids[i] = -1;
            success = 1;
            break;
        }
    }
    return success;
}

/**
 * Method to process the request - extract path, prepare response
 * @param req
 * @param resp
 */
int process_request(char *req, char *resp) {
    char *path_rel, search_query[LENGTH_OF_PATH];
    path_rel = extract_search_string(req);
    printf("Search requested: %s\n", path_rel);
    if (path_rel != NULL) {
        sprintf(search_query, "%s", path_rel);
    } else {
        search_query[0] = '\0';
    }
    free(path_rel);

    // preparing response to be sent
    return prepare_response(search_query, resp);
}

/**
 * Method to read a file into "**file_content"
 * @param path
 * @param file_content
 * @return long
 */
long prepare_response_html(char *search, char **file_content) {
    if (strlen(search) == 0) {

    }
    MYSQL *connection = mysql_init(NULL);
    db_connect(connection);
    MYSQL_RES *result = db_search(connection, search);
    *file_content = (char *) malloc(10 * BUFSIZ * sizeof(char));
    MYSQL_ROW row;
    sprintf(*file_content, "<html><head><title>Search Results</title><head><body style=\""
            "width: 600px;"
            "margin: 50px auto;"
            "font-family: monospace;\">"
            "<form action=\"/index.html\" method=\"get\" style=\"font-size: 20px; margin-bottom: 20px;\">\n"
            "  Search: <input type=\"text\" name=\"search\" style=\"font-size: 20px;\">"
            "  <button type=\"submit\" style=\"padding: 7px;\n"
            "    vertical-align: top;\n"
            "    background: #fff;\n"
            "    border: 1px #999 dashed;\n"
            "    cursor: pointer;\">Submit</button>\n"
            "</form>"
    );
    while ((row = mysql_fetch_row(result)) != NULL) {
        sprintf(*file_content, "%s<h3><a  href=\"%s\"><b>%s</b></a></h3>", *file_content, row[1], row[1]);
        sprintf(*file_content, "%s<p>%s</p>", *file_content, row[2]);
    }
    sprintf(*file_content, "%s</body></html>", *file_content);
    mysql_free_result(result);
    mysql_close(connection);
    return strlen(*file_content);
}

/**
 * Method to prepare the response
 * @param path
 * @param resp
 */
int prepare_response(char *path, char *resp) {
    int res_len = 0;
    char *file_content;
    long input_file_size = prepare_response_html(path, &file_content);
    char path_404[LENGTH_OF_PATH];

    if (input_file_size >= 0) {
        sprintf(resp, "HTTP/1.1 200 OK\n");
        sprintf(resp, "%sContent-Length: %ld\n", resp, input_file_size);
        // set content type based on the path extension
        char *ext_name;
        ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));
        int ext_match_found = match_extension(path, &ext_name);

        if (ext_match_found >= 0 && strlen(ext_name) > 0)
            sprintf(resp, "%sContent-Type: text/html\n", resp);
        else
            sprintf(resp, "%sContent-Type: text/html\n", resp);
        free(ext_name);
        sprintf(resp, "%sConnection: close\n", resp);

        // in case of Binary files strlen can not be used
        // hence, use of memcpy for copying the content universally
        sprintf(resp, "%s\n", resp);
        res_len = strlen(resp) + input_file_size;
        memcpy(resp + strlen(resp), file_content, input_file_size);

        free(file_content);
        printf("Sending Response: 200 OK\n\n");
    } else {
        sprintf(path_404, "%s/404.html", root_path);
        sprintf(resp, "HTTP/1.1 404 NOT FOUND\n");
        sprintf(resp, "%sContent-Type: text/html\n", resp);
        sprintf(resp, "%sConnection: close\n", resp);
        sprintf(resp, "%sContent-Length: 0\n", resp);
        sprintf(resp, "%s\n%s", resp, file_content);
        free(file_content);
        res_len = strlen(resp);
        printf("Sending Response: 404 NOT FOUND\n");
    }
    // returning length of the response
    return res_len;
}

/**
 * Method to write the server log in a permanent file
 * @param req
 * @param resp
 */
void write_log(char *req, char *resp) {
    FILE *log_file;
    char log_path[LENGTH_OF_PATH];
    sprintf(log_path, "%s/server.log", root_path);
    if ((log_file = fopen(log_path, "a")) != 0) {
        fprintf(log_file, "------| REQUEST  |------------------------------------------------\n");
        fprintf(log_file, "%s\n", req);
        fprintf(log_file, "------| RESPONSE |------------------------------------------------\n");
        fprintf(log_file, "%s\n\n", resp);
        fclose(log_file);
    } else {
        printf("ERROR: Could not write to log file.\n");
    }
}

