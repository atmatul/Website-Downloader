#ifndef WEBSITE_DOWNLOADER_DATABASE_H
#define WEBSITE_DOWNLOADER_DATABASE_H

#include "includes.h"

#define DATABASE_HOST "localhost"
#define DATABASE_UNAME "root"
#define DATABASE_PASSWORD ""

void db_debug(MYSQL *connection) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    mysql_close(connection);
}

int db_connect(MYSQL *connection) {
    if (mysql_real_connect(connection, DATABASE_HOST,
                           DATABASE_UNAME, DATABASE_PASSWORD,
                           "network_lab", 0, NULL, 0) == NULL) {
        db_debug(connection);
        notify_error("Unable to connect to database.\n");
    }
    return EXIT_SUCCESS;
}

int db_insert_link(MYSQL *connection, const char *url) {
    char query[BUFSIZ];
    sprintf(query, "INSERT INTO Links (link) VALUES('%s');", url);
    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }
    return EXIT_SUCCESS;
}

#endif //WEBSITE_DOWNLOADER_DATABASE_H
