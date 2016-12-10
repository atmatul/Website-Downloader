#ifndef WEBSITE_DOWNLOADER_THREADMANAGE_H
#define WEBSITE_DOWNLOADER_THREADMANAGE_H
#include <pthread.h>
#include "config.h"

#define MAX_THREAD_NUM 4

struct sthread_data {
    int id;
    configuration config;
    char* url;
    char* pagelink;
    MYSQL* connection;
    int *db_singleton;
    pthread_mutex_t* lock;
    pthread_cond_t* condition;
    pthread_t *thread_pool;
};

typedef struct sthread_data thread_data;

int is_available(pthread_t thread_pool[]) {
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_pool[i] == NULL)
            return i;
    }
    return -1;
}

int is_only_thread(pthread_t thread_pool[]) {
    int is_only = -1;
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_pool[i] != NULL) is_only++;
        if (is_only > 0) break;
    }
    return is_only;
}

void wait_on_condition(pthread_t thread_pool[], int *db_singleton, pthread_cond_t *condition, pthread_mutex_t *lock) {
    pthread_mutex_lock(lock);
    if (is_only_thread(thread_pool) > 0) {
        while(!(*db_singleton))
            pthread_cond_wait(condition, lock);
    }
    *db_singleton = 0;
}

void free_resource(int *db_singleton, pthread_cond_t *condition, pthread_mutex_t *lock) {
    *db_singleton = 1;
    pthread_cond_signal(condition);
    pthread_mutex_unlock(lock);
}


#endif //WEBSITE_DOWNLOADER_THREADMANAGE_H
