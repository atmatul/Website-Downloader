#ifndef WEBSITE_DOWNLOADER_CONFIG_H_H
#define WEBSITE_DOWNLOADER_CONFIG_H_H

#include <openssl/ssl.h>
#include "ini/ini.h"
#include "includes.h"

/* The configuration structure to store all the configuration variables */
typedef struct {
    const char* root_save_path;         /* the path in the hard-drive to save the downloaded pages */
    const char* invalid_save_path;      /* the path in the hard-drive to save files with unresolved urls */
    char *host;                         /* hostname of the server to be fetched from */
    char *protocol;                     /* protocol of the server to be fetched from */
    char *page;                         /* page from where the download starts */
    int begin_at;                       /* beginning id of the page in database. if starting fresh begin_at = 1 */
    char *cert_location;                /* location of the ssl certificates for https */
    int timeout;                        /* http read timeout for the select loop */

    char *root_path;                    /* root path to serve the files in the search engine */

    char *db_host;                      /* hostname for the database connection */
    char *db_username;                  /* username for the database connection */
    char *db_password;                  /* password for the database connection */
    char *db_name;                      /* name of the database */
} configuration;

/**
 * The handler function to link all the configuration variables
 * available in the .ini configuration file
 * @param config
 * @param section
 * @param name
 * @param value
 * @return
 */
static int handler(void *config, const char *section, const char *name,
                   const char *value) {
    configuration *pconfig = (configuration *) config;

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
    } else if (MATCH("server", "timeout")) {
        pconfig->timeout = atoi(value);
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
