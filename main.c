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

    pthread_t thread_pool[MAX_THREAD_NUM] = {NULL};

    int db_singleton_val = 0;

    pthread_mutex_t lock;
    pthread_cond_t condition;
    if (pthread_mutex_init(&lock, NULL)) {
        notify_error("Unable to initialize mutex.\n");
    }
    if (pthread_cond_init(&condition, NULL)) {
        notify_error("Unable to initialize condition.\n");
    }

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
    int waiting = 0, init = 1;

    while (1) {
        memset(pagelink, 0, strlen(pagelink));

        db_fetch_link(connection, id, &pagelink);
        char *url;
        url = (char *) malloc(BUFSIZ * sizeof(char));
        if (pagelink == NULL) break;
        memcpy(url, pagelink, strlen(pagelink));
        url[strlen(pagelink)] = '\0';

        int tid_index;
        if ((tid_index = is_available(thread_pool)) >= 0) {
            thread_data *tdata = (thread_data *) malloc(sizeof(thread_data));
            tdata->id = id;
            tdata->config = config;
            tdata->url = (char *) malloc((strlen(url) + 1) * sizeof(char));
            strcpy(tdata->url, url);
            tdata->pagelink = (char *) malloc((strlen(pagelink) + 1) * sizeof(char));
            strcpy(tdata->pagelink, pagelink);
            tdata->connection = connection;
            tdata->lock = &lock;
            tdata->condition = &condition;
            tdata->db_singleton = &db_singleton_val;
            tdata->thread_pool = thread_pool;

            int ret = pthread_create(&(thread_pool[tid_index]), NULL, fetch_resource_url, tdata);
            if (!ret) {
                if (init) {
                    int init_id = id;
                    while (waiting < 5) {
                        id = db_fetch_next_id(connection, init_id);
                        if (id == -1) {
                            waiting++;
                            printf("Couldn't get a valid id from db. Waiting for %d sec.\n", waiting);
                            sleep(waiting);
                        } else {
                            break;
                        }
                    }
                    init = 0;
                } else {
                    id = db_fetch_next_id(connection, id);
                }
                continue;
            }
        }
        for (int i = 0; i < MAX_THREAD_NUM; i++) {
            if (thread_pool[i] != NULL) {
                void *ret_val;
                int ret = pthread_join(thread_pool[i], ret_val);
                if (!ret) {
                    thread_pool[i] = NULL;
                }
            }
        }
    }

    free(pagelink);
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    mysql_close(connection);
    return 0;
}