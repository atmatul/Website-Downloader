#ifndef WEBSITE_DOWNLOADER_THREADMANAGE_H
#define WEBSITE_DOWNLOADER_THREADMANAGE_H
#include <pthread.h>
#include "config.h"

#define MAX_THREAD_NUM 32

/* The data structure to be passed on to each thread */
struct sthread_data {
    int id;                         /* id of the page url in the database */
    char* url;                      /* url corresponding to the page */
    char* pagelink;                 /* url corresponding to the page - immutable */
    MYSQL* connection;              /* MySQL database connection parameter */
    int *db_singleton;              /* the atomic variable corresponding to the database connection singleton */
    pthread_mutex_t* lock;          /* mutex variable for thread synchronization */
    pthread_cond_t* condition;      /* conditional for thread synchronization */
    pthread_t *thread_pool;         /* reference to the main thread pool */
    struct timeval *start;          /* starting time of the thread */
};

typedef struct sthread_data thread_data;

/**
 * Check if resources are available for creation of a new thread
 * @param the thread_pool to be checked
 */
int is_available(pthread_t thread_pool[]) {
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_pool[i] == NULL)
            return i;
    }
    return -1;
}

/**
 * Check if the thread_pool contains only a single thread
 * @param thread_pool the thread_pool to be checked
 * @return
 */
int is_only_thread(pthread_t thread_pool[]) {
    int is_only = -1;
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_pool[i] != NULL) is_only++;
        if (is_only > 0) break;
    }
    return is_only;
}

/**
 * Implementation of a conditional mutex lock to use the sigleton database connection
 * @param thread_pool the thread_pool containg all the threads
 * @param db_singleton the atomic variable corresponding to the db connection
 * @param condition the condition used to synchronize the threads
 * @param lock the mutex variable used to synchronize the threads
 */
void wait_on_condition(pthread_t thread_pool[], int *db_singleton, pthread_cond_t *condition, pthread_mutex_t *lock) {
    pthread_mutex_lock(lock);
    if (is_only_thread(thread_pool) > 0) {
        while(!(*db_singleton)) {
            pthread_cond_wait(condition, lock);
        }
    }
    *db_singleton = 0;
}

/**
 * Function to unlock the database resource to be used by other threads
 * @param db_singleton the atomic variable corresponding to the db connection
 * @param condition the condition used to synchronize the threads
 * @param lock the mutex variable used to synchronize the threads
 */
void free_resource(int *db_singleton, pthread_cond_t *condition, pthread_mutex_t *lock) {
    *db_singleton = 1;
    pthread_mutex_unlock(lock);
    pthread_cond_signal(condition);
}

#endif //WEBSITE_DOWNLOADER_THREADMANAGE_H
