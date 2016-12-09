#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"
#include "lib/file_saver.h"
#include "lib/database.h"
#include "lib/config.h"
#include "lib/mthread.h"

int main(int argc, char *argv[]) {
    char *config_filename;
    configuration config;
    if (argc > 1) {
        config_filename = argv[1];
    } else {
        config_filename = "../config.ini";
    }
    if (ini_parse(config_filename, handler, &config) < 0) {
        notify_error("Unable to load config file.");
    }

    pthread_t thread_pool[MAX_THREAD_NUM] = { NULL };

    char *pagelink;
    pagelink = (char *) malloc(BUFSIZ * sizeof(char));

    char delete_command[BUFSIZ];
    sprintf(delete_command, "rm -rf %s/*", config.root_save_path);

    MYSQL *connection = mysql_init(NULL);
    db_connect(connection);

    system(delete_command);
    db_reset(connection);
    db_insert_link(connection, config.page);
    int id = 1;
    while (1) {
        char *content, *header;
        int content_size = 0;
        memset(pagelink, 0, strlen(pagelink));

        db_fetch_link(connection, id, &pagelink);
        char *url;
        url = (char *) malloc(BUFSIZ * sizeof(char));
        if (pagelink == NULL) break;
        memcpy(url, pagelink, strlen(pagelink));
        url[strlen(pagelink)] = '\0';
        char* ext_name = (char *) malloc(MAX_EXT_LENGTH * sizeof(char));
        if (match_extension(url, &ext_name) != 0) {
            int tid_index;
            if ((tid_index = is_available(thread_pool)) >= 0) {
                thread_data *tdata = (thread_data *) malloc(sizeof(thread_data));
                tdata->id = id;
                tdata->config = config;
                tdata->url = (char *) malloc((strlen(url) + 1) * sizeof(char));
                strcpy(tdata->url, url);
                tdata->pagelink = (char *) malloc((strlen(pagelink) + 1) * sizeof(char));
                strcpy(tdata->pagelink, pagelink);
                int ret = pthread_create(&(thread_pool[tid_index]), NULL, fetch_resource_url, tdata);
                if (!ret) {
                    id = db_fetch_next_id(connection, id);
                    continue;
                }
            }
        }
        content_size = fetch_url(config.host, url, &header, &content);
        if (strlen(header) > 0) {
            int status_code = extract_response_code(header);
            while (status_code >= 300 && status_code < 400) {
                char *redirect_page;
                redirect_page = extract_redirect_location(header, config.host);
                if (redirect_page == NULL) break;
                else
                    memcpy(url, redirect_page, strlen(redirect_page));
                free(redirect_page);
                content_size = fetch_url(config.host, url, &header, &content);
            }
            if (!is_html(header)) {
                char *title = extract_title(content);
                if (strlen(title) > 0) {
                    db_insert_title(connection, title, id);
                    free(title);
                }
                link_extractor(connection, id, pagelink, content);
                tags_extractor(connection, id, content);
//                description_extractor(connection, id, content);
            }
            if (!file_save(config, pagelink, header, content, content_size)) {
                printf("(#%d) Saving: %s\n", id, pagelink);
            } else {
                printf("Error: Unable to save file.\n");
            }
        }
        free(content);
        free(header);

        for (int i = 0; i < MAX_THREAD_NUM; i++) {
            if (thread_pool[i] != NULL) {
                void *ret_val;
                int ret = pthread_join(thread_pool[i], ret_val);
                if (!ret) {
                    thread_pool[i] = NULL;
                }
            }
        }
        id = db_fetch_next_id(connection, id);
    }

    free(pagelink);
    mysql_close(connection);
    return 0;
}