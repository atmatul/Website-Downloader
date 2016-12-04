#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"
#include "lib/database.h"
#include "lib/config.h"

int main(int argc, char* argv[]) {
    char* config_filename;
    configuration config;
    if (argc > 1) {
        config_filename = argv[1];
    } else {
        config_filename = "../config.ini";
    }
    if (ini_parse(config_filename, handler, &config) < 0) {
        notify_error("Unable to load config file.");
    }

    char *pagelink;
    pagelink = (char *) malloc(BUFSIZ * sizeof(char));

    MYSQL* connection = mysql_init(NULL);
    db_connect(connection);
    db_reset(connection);
    db_insert_link(connection, config.page);
    int id = 1;

    while (1) {
        char *content, *header;
        memset(pagelink, 0, strlen(pagelink));
        db_fetch_link(connection, id, &pagelink);
        char *url;
        url = (char *) malloc(BUFSIZ * sizeof(char));
        if (pagelink == NULL) break;
        memcpy(url, pagelink, strlen(pagelink));
        url[strlen(pagelink)] = '\0';
        fetch_url(config.host, url, &header, &content);
        if (strlen(header) > 0) {
            if (!is_html(header)) {
                link_extractor(connection, pagelink, content);
            }
            char filepath[BUFSIZ];
            char *ext_name;
            ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));

            sprintf(filepath, "%s%s", config.root_save_path, pagelink);
            if (filepath[strlen(filepath) - 1] == '/') {
                sprintf(filepath, "%s%s", filepath, "index.html");
            }
            if (match_extension(filepath, &ext_name) == -1) {
                sprintf(filepath, "%s%s", filepath, ".html");
            }
            char cmd_mkdir[BUFSIZ];
            char dirpath[BUFSIZ];
            sprintf(dirpath, "%.*s", (int)(strrchr(filepath, '/') - filepath), filepath);
            sprintf(cmd_mkdir, "[ -d %s ] || mkdir -p %s", dirpath, dirpath);
            if (!system(cmd_mkdir)){
                file_write(filepath, content);
                printf("Wrote: %s\n", filepath);
            } else
                notify_error("Unable to write to file.");
            free(ext_name);
        }
        free(content);
        free(header);
        id = db_fetch_next_id(connection, id);
    }

    free(pagelink);
    mysql_close(connection);
    return 0;
}