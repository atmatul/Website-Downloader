#ifndef WEBSITE_DOWNLOADER_DOWNLOADER_H
#define WEBSITE_DOWNLOADER_DOWNLOADER_H

#include "includes.h"
#include "extractor.h"
#include "file_saver.h"
#include "mthread.h"
#include "config.h"
#include <openssl/ssl.h>

#define USERAGENT "NPLAB 1.0"

int create_socket(struct addrinfo *server_info) {
    int socket_id;
    if ((socket_id = socket(server_info->ai_family,
                            server_info->ai_socktype,
                            server_info->ai_protocol)) < 0) {
        notify_error("Unable to create socket.");
    }
    return socket_id;
}

struct addrinfo *resolve_ip_address(char *host, char *port) {
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(host, port, &hints, &server_info) != 0) {
        notify_error("Couldn't parse the host address");
    }

    return server_info;
}

char *build_get_query(char *host, char *page) {
    char *query;
    char *getpage;
    getpage = page;
    char *tpl = "GET /%s HTTP/1.0\n"
            "Host: %s\n"
            "User-Agent: %s\n"
            "Cache-Control: no-cache\n\n";
    if (getpage[0] == '/') {
        getpage = getpage + 1;
    }
    // -5 is to consider the %s %s %s in tpl and the ending \0
    query = (char *) malloc(strlen(host) + strlen(getpage) + strlen(USERAGENT) + strlen(tpl));
    sprintf(query, tpl, getpage, host, USERAGENT);
    return query;
}

int fetch_url_https(char *page, char **header, char **content) {
    extern configuration config;
    char *host = config.host;
    int result_id;
    char *http_header;
    char buffer[BUFSIZ + 1];
    char hostport[BUFSIZ];
    BIO *bio;
    SSL *ssl;
    SSL_CTX *ctx;

    ctx = SSL_CTX_new(SSLv23_client_method());

    if (ctx == NULL) {
        notify_error("Error loading trust store\n");
    }

    http_header = build_get_query(host, page);

    if (!SSL_CTX_load_verify_locations(ctx, config.cert_location, NULL)) {
        SSL_CTX_free(ctx);
        notify_error("Error loading trust store\n");
    }

    bio = BIO_new_ssl_connect(ctx);

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    sprintf(hostport, "%s:%s", host, config.protocol);
    BIO_set_conn_hostname(bio, hostport);

    if (BIO_do_connect(bio) <= 0) {
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        notify_error("Error attempting to connect\n");
    }

    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        fprintf(stderr, "Certificate verification error: %li\n", SSL_get_verify_result(ssl));
        exit(EXIT_FAILURE);
    }

    BIO_write(bio, http_header, strlen(http_header));

    // Receiving the page
    memset(buffer, 0, BUFSIZ + 1);
    int htmlstart = 0;
    int pagesize = 0;
    char *htmlcontent;

    fd_set observer;
    int bio_fd;
    BIO_get_fd(bio, &bio_fd);
    if (bio_fd == -1) {
        printf(ANSI_COLOR_RED "Unable to get the fd for the BIO\n" ANSI_COLOR_RESET);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);

        return pagesize;
    }

    struct timeval timeout;

    while (1) {
        timeout.tv_sec = config.timeout;
        timeout.tv_usec = 0;
        FD_ZERO(&observer);
        FD_SET(bio_fd, &observer);
        int ready = select(bio_fd + 1, &observer, (fd_set *) 0, (fd_set *) 0, &timeout);

        if (ready < 0) {
            printf(ANSI_COLOR_RED "Malfunction in select loop. Skipping " ANSI_COLOR_RESET);
            printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, page);
            break;
        } else if (ready == 0) {
            printf(ANSI_COLOR_RED "Select timed out. Skipping " ANSI_COLOR_RESET);
            printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, page);
            break;
        } else {
            if (FD_ISSET(bio_fd, &observer)) {
                if ((result_id = BIO_read(bio, buffer, BUFSIZ)) > 0) {
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
                        memset((*content + pagesize), 0, result_id);
                        memcpy((*content + pagesize), htmlcontent, result_id);
                        pagesize += result_id;
                    }
                    memset(buffer, 0, result_id);
                } else {
                    break;
                }
            }
        }
    }

    if (pagesize == 0) {
        printf(ANSI_COLOR_RED "ERROR: Receiving data: %s\n" ANSI_COLOR_RESET, page);
    }

    if (*header != NULL && !is_html(*header)) {
        *content = (char *) realloc(*content, (pagesize + 1) * sizeof(char));
        memset((*content + pagesize), '\0', 1);
    }
    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    return pagesize;
}


int fetch_url(char *page, char **header, char **content) {
    extern configuration config;
    char *host = config.host;
    char *port = (strcmp(config.protocol, "http") == 0) ? "80" : "443";
    int socket_id;
    int result_id;
    struct addrinfo *server_info;
    char *http_header;
    char buffer[BUFSIZ + 1];

    server_info = resolve_ip_address(host, port);
    socket_id = create_socket(server_info);

    if (connect(socket_id, server_info->ai_addr, server_info->ai_addrlen) < 0) {
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

    // Receiving the page
    memset(buffer, 0, BUFSIZ + 1);
    int htmlstart = 0;
    int pagesize = 0;
    char *htmlcontent;
    fd_set observer;

    struct timeval timeout;

    while (1) {
        timeout.tv_sec = config.timeout;
        timeout.tv_usec = 0;
        FD_ZERO(&observer);
        FD_SET(socket_id, &observer);
        int ready = select(socket_id + 1, &observer, (fd_set *) 0, (fd_set *) 0, &timeout);

        if (ready < 0) {
            printf(ANSI_COLOR_RED "Malfunction in select loop. Skipping " ANSI_COLOR_RESET);
            printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, page);
            break;
        } else if (ready == 0) {
            printf(ANSI_COLOR_RED "Select timed out. Skipping " ANSI_COLOR_RESET);
            printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, page);
            break;
        } else {
            if (FD_ISSET(socket_id, &observer)) {
                if ((result_id = recv(socket_id, buffer, BUFSIZ, 0)) > 0) {
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
                        memset((*content + pagesize), 0, result_id);
                        memcpy((*content + pagesize), htmlcontent, result_id);
                        pagesize += result_id;
                    }
                    memset(buffer, 0, result_id);
                } else {
                    break;
                }
            }
        }
    }

    if (pagesize == 0) {
        printf(ANSI_COLOR_RED "ERROR: Receiving data: %s\n" ANSI_COLOR_RESET, page);
    }

    if (*header != NULL && !is_html(*header)) {
        *content = (char *) realloc(*content, (pagesize + 1) * sizeof(char));
        memset((*content + pagesize), '\0', 1);
    }

    free(http_header);
    free(server_info);
    close(socket_id);
    return pagesize;
}

void *fetch_resource_url(void *data) {
    extern configuration config;
    thread_data *tdata = (thread_data *) data;
    char *header, *content;
    int content_size = (strcmp(config.protocol, "https") == 0) ?
                       fetch_url_https(tdata->url, &header, &content) :
                       fetch_url(tdata->url, &header, &content);
    char template[BUFSIZ] = "# %-6d - " ANSI_COLOR_YELLOW " %6d %s   " ANSI_COLOR_RESET
            ANSI_COLOR_BLUE "Downloaded: %-60s" ANSI_COLOR_RESET ANSI_COLOR_YELLOW "    in "
            ANSI_COLOR_RESET ANSI_COLOR_BLUE "%ld %s\n" ANSI_COLOR_RESET;
    char output[BUFSIZ];

    if (content_size && strlen(header) > 0) {
        int status_code = extract_response_code(header);
        while (status_code >= 300 && status_code < 400) {
            char *redirect_page;
            redirect_page = extract_redirect_location(header, config.host);
            if (redirect_page == NULL) break;
            else
                memcpy(tdata->url, redirect_page, strlen(redirect_page));
            free(redirect_page);
            content_size = (strcmp(config.protocol, "https") == 0) ?
                           fetch_url_https(tdata->url, &header, &content) :
                           fetch_url(tdata->url, &header, &content);
        }
        if (!is_html(header)) {
            char *title = extract_title(content);
            if (strlen(title) > 0) {
                wait_on_condition(tdata->thread_pool, tdata->db_singleton, tdata->condition, tdata->lock);
                db_insert_title(tdata->connection, title, tdata->id);
                free_resource(tdata->db_singleton, tdata->condition, tdata->lock);
                free(title);
            }
            wait_on_condition(tdata->thread_pool, tdata->db_singleton, tdata->condition, tdata->lock);
            link_extractor(tdata->connection, tdata->id, tdata->pagelink, content);
            free_resource(tdata->db_singleton, tdata->condition, tdata->lock);

            wait_on_condition(tdata->thread_pool, tdata->db_singleton, tdata->condition, tdata->lock);
            tags_extractor(tdata->connection, tdata->id, content);
            free_resource(tdata->db_singleton, tdata->condition, tdata->lock);
//                description_extractor(connection, id, content);
        }
        if (!file_save(config, tdata->pagelink, header, content, content_size)) {
            struct timeval *end = (struct timeval *) malloc(sizeof(struct timeval));
            gettimeofday(end, NULL);
            if (tdata->start && end) {
                long int time = 0;
                time += (end->tv_sec - tdata->start->tv_sec) * 1000;
                time += (end->tv_usec - tdata->start->tv_usec) / 1000;
                sprintf(output, template, tdata->id, content_size < 1024 ? content_size : content_size / 1024,
                        content_size < 1024 ? "B " : "KB", tdata->pagelink, time > 1000 ? time / 1000 : time,
                        time > 1000 ? "s" : "ms");
                printf("%s", output);
            }
            free(end);
            free(tdata->start);
        } else {
            printf("Error: Unable to save file.\n");
        }
    }

    free(tdata->url);
    free(tdata->pagelink);
    if (content_size) {
        free(header);
        free(content);
    }
    free(tdata);
    return NULL;
}

#endif //WEBSITE_DOWNLOADER_DOWNLOADER_H
