#ifndef WEBSITE_DOWNLOADER_DOWNLOADER_H
#define WEBSITE_DOWNLOADER_DOWNLOADER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "includes.h"
#include "extractor.h"
#include "file_saver.h"
#include "mthread.h"

#define PORT 80
#define USERAGENT "NPLAB 1.0"

int create_socket() {
    int socket_id;
    if ((socket_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        notify_error("Unable to create socket.");
    }
    return socket_id;
}

char *resolve_ip_address(char *host) {
    struct hostent *hent;
    int iplen = 15;
    char *ip = (char *) malloc(iplen + 1);
    memset(ip, 0, iplen + 1);
    if ((hent = gethostbyname(host)) == NULL) {
        notify_error("Unable to resolve IP");
    }
    if (inet_ntop(AF_INET, (void *) hent->h_addr_list[0], ip, iplen) == NULL) {
        notify_error("Unable to resolve host");
    }
    return ip;
}

char *build_get_query(char *host, char *page) {
    char *query;
    char *getpage;
    getpage = page;
    char *tpl = "GET /%s HTTP/1.0\nHost: %s\nUser-Agent: %s\n\n";
    if (getpage[0] == '/') {
        getpage = getpage + 1;
    }
    // -5 is to consider the %s %s %s in tpl and the ending \0
    query = (char *) malloc(strlen(host) + strlen(getpage) + strlen(USERAGENT) + strlen(tpl) - 5);
    sprintf(query, tpl, getpage, host, USERAGENT);
    return query;
}

int fetch_url(char *host, char* page, char **header, char **content) {
    struct sockaddr_in *server_addr;
    int socket_id;
    int result_id;
    char *ip;
    char *http_header;
    char buffer[BUFSIZ + 1];

    socket_id = create_socket();
    ip = resolve_ip_address(host);

    server_addr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in *));
    server_addr->sin_family = AF_INET;
    result_id = inet_pton(AF_INET, ip, (void *) (&(server_addr->sin_addr.s_addr)));
    if (result_id < 0) {
        notify_error("Can't set server address.");
    } else if (result_id == 0) {
        notify_error("IP address invalid.");
    }

    server_addr->sin_port = htons(PORT);

    if (connect(socket_id, (struct sockaddr *) server_addr, sizeof(struct sockaddr)) < 0) {
        notify_error("Could not connect");
    }
    http_header = build_get_query(host, page);

    // Send the query to the server
    int sent = 0;
    while (sent < strlen(http_header)) {
        result_id = send(socket_id, http_header + sent, strlen(http_header) - sent, 0);
        if (result_id == -1) {
            notify_error("Can't send query");
        }
        sent += result_id;
    }

    int ishtml;
    // Receiving the page
    memset(buffer, 0, sizeof(buffer));
    int htmlstart = 0;
    int pagesize = 0;
    char *htmlcontent;
    while ((result_id = recv(socket_id, buffer, BUFSIZ, 0)) > 0) {
        if (htmlstart == 0) {
            htmlcontent = strstr(buffer, "\r\n\r\n");
            if (htmlcontent != NULL) {
                htmlstart = 1;
                htmlcontent += 4;
                int header_size = (int) (htmlcontent - buffer);
                *header = (char *) malloc((header_size + 1) * sizeof(char));
                memcpy(*header, buffer, header_size);
                *(*header + header_size) = '\0';
                pagesize += result_id - header_size;
                *content = (char *) malloc(pagesize * sizeof(char));
                memset(*content, 0, pagesize);
                memcpy(*content, htmlcontent, pagesize);
            }
        } else {
            htmlcontent = buffer;
            *content = (char *) realloc(*content, (pagesize + result_id) * sizeof(char));
            memset((*content+pagesize), 0, result_id);
            memcpy((*content+pagesize), htmlcontent, result_id);
            pagesize += result_id;
        }
        memset(buffer, 0, result_id);
    }
    if (pagesize == 0) {
        printf("ERROR: Receiving data: %s\n", page);
    }

    if (*header != NULL && !is_html(*header)) {
        *content = (char *) realloc(*content, (pagesize + 1) * sizeof(char));
        memset((*content+pagesize), '\0', 1);
    }

    free(http_header);
    free(server_addr);
    free(ip);
    close(socket_id);
    return pagesize;
}

void *fetch_resource_url(void *data) {
    thread_data *tdata = (thread_data *) data;
    char *header, *content;
    int content_size = fetch_url(tdata->config.host, tdata->url, &header, &content);
    if (strlen(header) > 0) {
        int status_code = extract_response_code(header);
        while (status_code >= 300 && status_code < 400) {
            char *redirect_page;
            redirect_page = extract_redirect_location(header, tdata->config.host);
            if (redirect_page == NULL) break;
            else
                memcpy(tdata->url, redirect_page, strlen(redirect_page));
            free(redirect_page);
            content_size = fetch_url(tdata->config.host, tdata->url, &header, &content);
        }
        if (is_html(header)) {
            if (!file_save(tdata->config, tdata->pagelink, header, content, content_size)) {
                printf("(#%d) Saving: %s\n", tdata->id, tdata->pagelink);
            } else {
                printf("Error: Unable to save file.\n");
            }
        }
    }

    free(tdata->url);
    free(tdata->pagelink);
    free(header);
    free(content);
    free(tdata);
    return NULL;
}

#endif //WEBSITE_DOWNLOADER_DOWNLOADER_H
