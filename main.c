#include "lib/includes.h"
#include "lib/extractor.h"
#include "lib/downloader.h"

#include <unistd.h>
#include <pthread.h>

configuration config;

#define CNTRIDLE 5
#define IDLETIME 1000UL * 500UL
#define NUMTHREADS 8


/**
 * Thread worker function.
 * @param tid thread ID
 * @return
 */
void *t_worker(void *tid) {
    unsigned int threadID = *(unsigned int *) tid;
    unsigned int idleCounter = 0;

    char *pagelink;
    pagelink = (char *) malloc(BUFSIZ * sizeof(char));

    /* Setup MYSQL connection */
    MYSQL *connection = mysql_init(NULL);
    db_connect(connection);

    /* wait some random time before starting the other threads */
    if (threadID > 0)
        usleep(1000000UL + rand() % 1000000UL * 1UL);

    /* Get first ID */
    int id = db_fetch_next_id(connection, 0);

    printf(ANSI_COLOR_GREEN "%s: starting thread [%d/%d]\n" ANSI_COLOR_RESET, __func__, threadID + 1, NUMTHREADS);

    while (1) {
        /* Fetch the link with the given id into pagelink */
        db_fetch_link(connection, id, &pagelink);

        if (pagelink != NULL) {
            char *url;
            url = (char *) malloc(BUFSIZ * sizeof(char));

            idleCounter = 0;
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
            tdata->start = (struct timeval *) malloc(sizeof(struct timeval));
            gettimeofday(tdata->start, NULL);

            fetch_resource_url(tdata);

            id = db_fetch_next_id(connection, id);

            memset(pagelink, 0, strlen(pagelink));
        } else {
            idleCounter++;

            if (idleCounter == 1)
                printf(ANSI_COLOR_GREEN "%s [%d/%d]:\t is idling...\n" ANSI_COLOR_RESET, __func__, threadID + 1,
                       NUMTHREADS);

            usleep(IDLETIME);

            if (idleCounter == CNTRIDLE) {
                printf(ANSI_COLOR_GREEN "%s [%d/%d]:\tStopping\n" ANSI_COLOR_RESET, __func__, threadID + 1, NUMTHREADS);

                /* clean up */
                mysql_close(connection);

                return NULL; //pthread_exit( NULL );
            }
        }
    }
}

/**
 * The main function.
 * @param argc the number of arguments passed through command line
 * @param argv the command line arguments accessible through a char*[]
 * @return
 */
int main(int argc, char *argv[]) {
    printf("%s: starting...\n", __func__);

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

    /* Reset? */
    if (config.begin_at == 1) {
        /* Form the command to delete the files in the html save path */
        char delete_command[BUFSIZ];
        sprintf(delete_command, "rm -rf %s/*", config.root_save_path);
        system(delete_command);

        /* Initialize the MySQL connection, reset the database, insert the starting link */
        MYSQL *connection = mysql_init(NULL);
        db_connect(connection);
        db_reset(connection);
        db_insert_link(connection, config.page);
        mysql_close(connection);
    }

    /* Check for threading support */
    if (mysql_thread_safe() == 0) {
        printf(ANSI_COLOR_YELLOW "mysql client library is not compiled as thread-safe => threading will not be used!\n" ANSI_COLOR_RESET);

        unsigned int threadID = 0;
        ( *t_worker )(&threadID);
    } else {
        /* Start threads */
        pthread_t threadMain[NUMTHREADS];
        unsigned int threadIDs[NUMTHREADS] = {0};
        int tid;

        for (tid = 0; tid < NUMTHREADS; tid++) {
            threadIDs[tid] = tid;
//			printf( ANSI_COLOR_GREEN "%s: starting thread [%d/%d]\n" ANSI_COLOR_RESET, __func__, tid + 1, NUMTHREADS );
            if (pthread_create(&threadMain[tid], NULL, t_worker, &threadIDs[tid]))
                perror(ANSI_COLOR_RED "Error creating thread\n" ANSI_COLOR_RESET);
        }

        for (tid = 0; tid < NUMTHREADS; tid++)
            pthread_join(threadMain[tid], NULL);
    }

    printf("%s: Finished!\n", __func__);
    return 0;
}
