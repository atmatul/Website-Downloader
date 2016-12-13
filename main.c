#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"

/**
 * The main fetcher function - initializes all the variables
 * and passes them to individual threads. Threads are also
 * joined inside the main function.
 * @param argc the number of arguments passed through command line
 * @param argv the command line arguments accessible through a char*[]
 * @return
 */
int main(int argc, char *argv[]) {
    char *config_filename;
    extern configuration config;

    /* Check if the configuration file is passed through command line */
    if (argc > 1) {
        config_filename = argv[1];
    } else {
        config_filename = "../config.ini";
    }

    /* Parse the .ini file using the configuration handler */
    if (ini_parse(config_filename, handler, &config) < 0) {
        notify_error("Unable to load config file.");
    }

    /* Initialize the OpenSSL library */
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_library_init();

    /* Initialize the thread-pool, data-pool, condition and mutexes */
    pthread_t thread_pool[MAX_THREAD_NUM] = {NULL};
    thread_data* thread_data_pool[MAX_THREAD_NUM] = {NULL};
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

    /* Form the command to delete the files in the html save path */
    char delete_command[BUFSIZ];
    sprintf(delete_command, "rm -rf %s/*", config.root_save_path);

    /* Initialize the MySQL connection, reset the database, insert the starting link */
    MYSQL *connection = mysql_init(NULL);
    db_connect(connection);
    /* Initialize the starting id */
    int id = config.begin_at;
    if (id == 1) {
        system(delete_command);
        db_reset(connection);
        db_insert_link(connection, config.page);
    }

    int waiting, init = 1;

    while (1) {
        /* Check if any free thread indexes available in the thread-pool */
        int tid_index;
        if (init != 2 && (tid_index = is_available(thread_pool)) >= 0) {
            memset(pagelink, 0, strlen(pagelink));
            /* Fetch the link with the given id into pagelink */
            db_fetch_link(connection, id, &pagelink);
            char *url;
            url = (char *) malloc(BUFSIZ * sizeof(char));
            if (pagelink == NULL) break;
            memcpy(url, pagelink, strlen(pagelink));
            url[strlen(pagelink)] = '\0';
            /* Initialize the thread_data structure with the available values */
            thread_data *tdata = (thread_data *) malloc(sizeof(thread_data));
            tdata->id = id;
            tdata->url = (char *) malloc((strlen(url) + 1) * sizeof(char));
            strcpy(tdata->url, url);
            tdata->pagelink = (char *) malloc((strlen(pagelink) + 1) * sizeof(char));
            strcpy(tdata->pagelink, pagelink);
            tdata->connection = connection;
            tdata->lock = &lock;
            tdata->condition = &condition;
            tdata->db_singleton = &db_singleton_val;
            tdata->thread_pool = thread_pool;
            tdata->start = (struct timeval *) malloc(sizeof(struct timeval));
            gettimeofday(tdata->start, NULL);
            thread_data_pool[tid_index] = tdata;
            /* Create the thread and pass on the thread_data structure as the function variable */
            int ret = pthread_create(&(thread_pool[tid_index]), NULL, fetch_resource_url, tdata);
            if (!ret) {
                if (init) init = 2;
                int init_id = id;
                /* Wait until the next id is available in the database */
                while (waiting < 5) {
                    id = db_fetch_next_id(connection, init_id);
                    if (id == -1) {
                        waiting++;
                        printf(ANSI_COLOR_YELLOW "Couldn't get a valid id from db. Waiting for %d sec.\n"
                                       ANSI_COLOR_RESET, waiting);
                        sleep(waiting);
                    } else {
                        break;
                    }
                }
                continue;
            }
        }
        /* Join the finished threads from the thread pool */
        for (int i = 0; i < MAX_THREAD_NUM; i++) {
            if (thread_pool[i] != NULL) {
                void *ret_val;
                int ret = pthread_join(thread_pool[i], ret_val);
                if (!ret) {
                    if (init) init = 0;
                    thread_pool[i] = NULL;
                    thread_data_pool[i] = NULL;
                }
            }
        }
    }

    /* Free up the allocated memory and close the database connection */
    free(pagelink);
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);

    mysql_close(connection);
    return 0;
}