#ifndef WEBSITE_DOWNLOADER_CONFIG_H_H
#define WEBSITE_DOWNLOADER_CONFIG_H_H

#include "ini/ini.h"
#include "includes.h"

typedef struct {
    const char* root_save_path;
    const char* invalid_save_path;
    char *host;
    char *protocol;
    char *page;
    int begin_at;
    char *cert_location;

    char *root_path;

    char *db_host;
    char *db_username;
    char *db_password;
    char *db_name;
} configuration;

static int handler(void *user, const char *section, const char *name,
                   const char *value) {
    configuration *pconfig = (configuration *) user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("storage", "save_location")) {
        pconfig->root_save_path = strdup(value);
    } else if (MATCH("storage", "invalid_save_location")) {
        pconfig->invalid_save_path = strdup(value);
    } else if (MATCH("server", "host")) {
        pconfig->host = strdup(value);
    } else if (MATCH("server", "protocol")) {
        pconfig->protocol = strdup(value);
    } else if (MATCH("server", "start_page")) {
        pconfig->page = strdup(value);
    } else if (MATCH("server", "begin_at")) {
        pconfig->begin_at = atoi(value);
    } else if (MATCH("server", "cert")) {
        pconfig->cert_location = strdup(value);
    } else if (MATCH("search_engine", "root_path")) {
        pconfig->root_path = strdup(value);
    } else if (MATCH("database", "host")) {
        pconfig->db_host = strdup(value);
    } else if (MATCH("database", "username")) {
        pconfig->db_username = strdup(value);
    } else if (MATCH("database", "password")) {
        pconfig->db_password = strdup(value);
    } else if (MATCH("database", "name")) {
        pconfig->db_name = strdup(value);
    } else {
        return EXIT_FAILURE;  /* unknown section/name, error */
    }
    return EXIT_SUCCESS;
}

#endif //WEBSITE_DOWNLOADER_CONFIG_H_H
