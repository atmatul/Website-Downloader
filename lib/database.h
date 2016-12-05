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

    if (mysql_query(connection, "DROP TABLE Ext_Links;")) {
        db_debug(connection);
        notify_error("Unable to drop old table.\n");
    }

    if (mysql_query(connection, "CREATE TABLE Links ("
            "id int NOT NULL AUTO_INCREMENT,"
            "link varchar(1023) NOT NULL,"
            "occurence int DEFAULT 1,"
            "tags text,"
            "PRIMARY KEY (id),"
            "UNIQUE (link)"
            ");")) {
        db_debug(connection);
        notify_error("Unable to re-create table.\n");
    }

    if (mysql_query(connection, "CREATE TABLE Ext_Links ("
            "id int NOT NULL AUTO_INCREMENT,"
            "link varchar(1023) NOT NULL,"
            "occurence int DEFAULT 1,"
            "PRIMARY KEY (id),"
            "UNIQUE (link)"
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

int db_add_tags(MYSQL* connection, int id, char* tags) {
    char query[2*BUFSIZ];

    sprintf(query, "UPDATE Links "
            "SET tags='%s' "
            "WHERE id='%d';", tags, id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }
    return EXIT_SUCCESS;
}

int db_insert_unique_link(MYSQL *connection, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Links (link, occurence) "
            " VALUES ('%s', 1) "
            " ON DUPLICATE KEY UPDATE occurence = occurence + 1;", url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }
    return EXIT_SUCCESS;
}

int db_insert_external_link(MYSQL *connection, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Ext_Links (link, occurence) "
            " VALUES ('%s', 1) "
            " ON DUPLICATE KEY UPDATE occurence = occurence + 1;", url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }
    return EXIT_SUCCESS;
}

int db_fetch_next_id(MYSQL *connection, int id) {
    char query[BUFSIZ];

    sprintf(query, "SELECT id FROM Links WHERE id > '%d' ORDER BY id ASC LIMIT 1;", id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }

    int new_id = id + 1;

    MYSQL_RES *result = mysql_store_result(connection);
    if (mysql_num_rows(result) == 1) {
        MYSQL_ROW link_row = mysql_fetch_row(result);
        if (link_row[0]) {
            new_id = atoi(link_row[0]);
        }
    }
    mysql_free_result(result);

    return new_id;
}


int db_fetch_link(MYSQL *connection, int id, char **link) {
    char query[BUFSIZ];

    sprintf(query, "SELECT link FROM Links WHERE id = '%d' LIMIT 1;", id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }

    MYSQL_RES *result = mysql_store_result(connection);
    if (mysql_num_rows(result) == 1) {
        MYSQL_ROW link_row = mysql_fetch_row(result);
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
