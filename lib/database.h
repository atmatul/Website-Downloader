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

int db_reset(MYSQL *connection) {

    if (mysql_query(connection, "DROP TABLE Links;")) {
        db_debug(connection);
        notify_error("Unable to drop old table.\n");
    }

    if (mysql_query(connection, "CREATE TABLE Links ("
                    "id int NOT NULL AUTO_INCREMENT,"
                    "link varchar(1023) NOT NULL,"
                    "PRIMARY KEY (id)"
                    ");")) {
        db_debug(connection);
        notify_error("Unable to re-create table.\n");
    }
    return EXIT_SUCCESS;

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


int db_insert_unique_link(MYSQL *connection, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Links (link) "
            "SELECT * FROM (SELECT '%s') AS tmp"
            "    WHERE NOT EXISTS ("
            "            SELECT link FROM Links WHERE link = '%s'"
            "    ) LIMIT 1;", url, url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }
    return EXIT_SUCCESS;
}

int db_fetch_link(MYSQL* connection, int id, char** link) {
    char query[BUFSIZ];

    sprintf(query, "SELECT link FROM Links WHERE id = '%d' LIMIT 1;", id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }

    MYSQL_RES *result = mysql_store_result(connection);
    MYSQL_ROW link_row = mysql_fetch_row(result);
    if (link_row != NULL) {
        if (link_row[0]) {
            strcpy(*link, link_row[0]);
        }
    } else {
        *link = NULL;
    }
    mysql_free_result(result);

    return EXIT_SUCCESS;
}
#endif //WEBSITE_DOWNLOADER_DATABASE_H
