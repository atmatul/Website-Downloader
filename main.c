#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"
#include "lib/database.h"

int main() {
    char* host = "hyperphysics.phy-astr.gsu.edu";
    char* page = "/hbase/pber.html";
    char* root_save_path = "/Users/kunal/Desktop/NP-Project/website-downloader/public_html";

    char *pagelink;
    pagelink = (char *) malloc(BUFSIZ * sizeof(char));

    MYSQL* connection = mysql_init(NULL);
    db_connect(connection);
    db_reset(connection);
    db_insert_link(connection, page);
    int id = 1;

    while (1) {
        char *content, *header;
        memset(pagelink, 0, strlen(pagelink));
        db_fetch_link(connection, id, &pagelink);
        char *url;
        url = (char *) malloc(BUFSIZ * sizeof(char));
        if (pagelink == NULL) break;
        memcpy(url, pagelink, strlen(pagelink));
        fetch_url(host, url, &header, &content);
        if (strlen(header) > 0) {
            if (!is_html(header)) {
                link_extractor(connection, pagelink, content);
            }
            char filepath[BUFSIZ];
            char *ext_name;
            ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));

            sprintf(filepath, "%s%s", root_save_path, pagelink);
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
        id++;
    }

    free(pagelink);
    mysql_close(connection);
    return 0;
}