#ifndef WEBSITE_DOWNLOADER_EXTRACTOR_H
#define WEBSITE_DOWNLOADER_EXTRACTOR_H

#include <ctype.h>
#include "includes.h"
#include "slre/slre.h"
#include "list.h"
#include "database.h"

#define NOT_VALID_URL -2
#define NOT_LOCAL_URL -1

int is_local_link(const char *link_url) {
    if (strlen(link_url) > 0) {
        char link_head[5];
        if (strlen(link_url) > 4) {
            memcpy(link_head, link_url, 4);
            link_head[4] = '\0';
        }
        if (link_url[0] != '/' &&
            ((strlen(link_url) > 4) &&
             (strcmp(link_head, "http") == 0 ||
              strcmp(link_head, "HTTP") == 0)))
            return NOT_LOCAL_URL;
        else
            return EXIT_SUCCESS;
    } else {
        return NOT_VALID_URL;
    }
}

int is_html(const char *header) {
    static const char *regex = "text/html";
    struct slre_cap caps[3];
    int j = 0, str_len = strlen(header);

    if (slre_match(regex, header + j, str_len - j, caps, 3, SLRE_IGNORE_CASE) > 0) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

int validate_link(char **link) {
    if ((strchr(*link, '(') != NULL) || (strchr(*link, ')') != NULL)) {
        return EXIT_FAILURE;
    }
    char *hash_location;
    if ((hash_location = strchr(*link, '#')) != NULL) {
        *hash_location = '\0';
    }
    if ((*link)[strlen(*link) - 1] == '/') {
        strcat(*link, "index.html");
    }
    return EXIT_SUCCESS;
}

char *urlencode(char *url, char* table) {
    char* temp_url = url;
    int i;
    char* enc = (char *) malloc(BUFSIZ * sizeof(char));
    memset(enc, 0, BUFSIZ);
    for (i = 0; i < 256; i++) {
        table[i] = isalnum(i) || i == '*' || i == '-' || i == '.' || i == '/' || i == ':' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }

    for (i = 0; *temp_url; temp_url++) {
        if (table[*temp_url]) {
            enc[i] = table[*temp_url];
            i++;
        } else {
            int j = sprintf(enc + i, "%%%02X", *temp_url);
            i+= j;
        }
    }

    return enc;
}

void link_extractor(MYSQL *connection, const char *url, const char *markup) {
    char table[256] = {0};
    int i = 0;
    for (i = 0; i < 256; i++) {
        table[i] = isalnum(i) || i == '*' || i == '-' || i == '.' ||
                           i == '/' || i == ':' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }

    static const char *regex = "(src|href)=\"([^'\"<>]+)\"";
    struct slre_cap caps[4];
    int j = 0, str_len = strlen(markup);
    char current_dir[BUFSIZ];
    sprintf(current_dir, "%.*s", (int) (strrchr(url, '/') - url + 1), url);
    while (j < str_len &&
           (i = slre_match(regex, markup + j, str_len - j, caps, 4, SLRE_IGNORE_CASE)) > 0) {
        char *subpat;
        subpat = (char *) malloc((caps[1].len + 1) * sizeof(char));
        memcpy(subpat, caps[1].ptr, caps[1].len);
        subpat[caps[1].len] = '\0';
        char* encoded_link;
        if (is_local_link(subpat) == 0) {
            if (subpat[0] != '/') {
                char *sanitized_link;
                sanitized_link = (char *) malloc(BUFSIZ * sizeof(char));
                sprintf(sanitized_link, "%s%s", current_dir, subpat);
                if (!validate_link(&sanitized_link)) {
                    encoded_link = urlencode(sanitized_link, table);
                    db_insert_unique_link(connection, encoded_link);
                    free(encoded_link);
                }
                free(sanitized_link);
            } else {
                if (!validate_link(&subpat)) {
                    encoded_link = urlencode(subpat, table);
                    db_insert_unique_link(connection, encoded_link);
                    free(encoded_link);
                }
            }
        } else {
            encoded_link = urlencode(subpat, table);
            db_insert_external_link(connection, encoded_link);
            free(encoded_link);
        }
        free(subpat);
        j += i;
    }
}

#endif //WEBSITE_DOWNLOADER_EXTRACTOR_H
