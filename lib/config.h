#ifndef WEBSITE_DOWNLOADER_CONFIG_H_H
#define WEBSITE_DOWNLOADER_CONFIG_H_H

#include "ini/ini.h"
#include "includes.h"

typedef struct {
    const char* root_save_path;
    const char* invalid_save_path;
    char *host;
    char *page;
    char *root_path;
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
    } else if (MATCH("server", "start_page")) {
        pconfig->page = strdup(value);
    } else if (MATCH("search_engine", "root_path")) {
        pconfig->root_path = strdup(value);
    } else {
        return EXIT_FAILURE;  /* unknown section/name, error */
    }
    return EXIT_SUCCESS;
}

#endif //WEBSITE_DOWNLOADER_CONFIG_H_H
