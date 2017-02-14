#ifndef WEBSITE_DOWNLOADER_THREADMANAGE_H
#define WEBSITE_DOWNLOADER_THREADMANAGE_H

#include "config.h"


/* The data structure to be passed on to each thread */
struct sthread_data {
    int id;                         /* id of the page url in the database */
    char *url;                      /* url corresponding to the page */
    char *pagelink;                 /* url corresponding to the page - immutable */
    MYSQL *connection;              /* MySQL database connection parameter */
    struct timeval *start;          /* starting time of the thread */
};

typedef struct sthread_data thread_data;


#endif //WEBSITE_DOWNLOADER_THREADMANAGE_H
