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
};

typedef struct sthread_data thread_data;

int is_available(pthread_t thread_pool[]) {
    for (int i = 0; i < MAX_THREAD_NUM; i++) {
        if (thread_pool[i] == NULL)
            return i;
    }
    return -1;
}

#endif //WEBSITE_DOWNLOADER_THREADMANAGE_H
