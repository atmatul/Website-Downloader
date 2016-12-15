#ifndef WEBSITE_DOWNLOADER_DATABASE_H
#define WEBSITE_DOWNLOADER_DATABASE_H

#include "includes.h"
#include "config.h"

/**
 * Helper function to print the recent errors stored in the connection
 * @param connection the MySQL connection parameter
 */
void db_debug(MYSQL *connection) {
    printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, mysql_error(connection));
}

/**
 * Resets the database deleting all the values
 * Drops all the tables and creates them again
 * @param connection the MySQL connection parameter
 * @return
 */
int db_reset(MYSQL *connection) {

    if (mysql_query(connection, "DROP TABLE IF EXISTS Link_Maps;")) {
        db_debug(connection);
        notify_error("Unable to drop old table.\n");
    }

    if (mysql_query(connection, "DROP TABLE IF EXISTS Links;")) {
        db_debug(connection);
        notify_error("Unable to drop old table.\n");
    }

    if (mysql_query(connection, "DROP TABLE IF EXISTS Ext_Links;")) {
        db_debug(connection);
        notify_error("Unable to drop old table.\n");
    }

    if (mysql_query(connection, "CREATE TABLE Links ("
            "id int NOT NULL AUTO_INCREMENT,"
            "link varchar(1023) NOT NULL,"
            "title varchar(1023),"
            "occurence int DEFAULT 1,"
            "tags text CHARACTER SET 'latin1',"
            "PRIMARY KEY (id),"
            "UNIQUE (link),"
            "FULLTEXT (tags)"
            ");")) {
        db_debug(connection);
        notify_error("Unable to re-create table.\n");
    }

    if (mysql_query(connection, "CREATE TABLE Link_Maps ("
            "id int NOT NULL AUTO_INCREMENT,"
            "from_id int NOT NULL,"
            "to_id int NOT NULL,"
            "times int DEFAULT 1,"
            "PRIMARY KEY (id),"
            "FOREIGN KEY (from_id) REFERENCES Links(id),"
            "FOREIGN KEY (to_id) REFERENCES Links(id),"
            "UNIQUE (from_id, to_id)"
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

/**
 * Connect to the database using the database configuration parameters passed
 * onto the configuration structure. Requires username, password, host and the name
 * of the database to connect to
 * @param connection the MySQL connection parameter
 * @return
 */
int db_connect(MYSQL *connection) {
    extern configuration config;
    if (mysql_real_connect(connection, config.db_host,
                           config.db_username, config.db_password,
                           config.db_name, 0, NULL, 0) == NULL) {
        db_debug(connection);
        notify_error("Unable to connect to database.\n");
    }
    return EXIT_SUCCESS;
}

/**
 * Insert the link url without any checks. This method is unsafe and might
 * duplicate values in the table if proper care is not taken
 * @param connection the MySQL connection parameter
 * @param url the url to be insert into the table
 * @return
 */
int db_insert_link(MYSQL *connection, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Links (link) VALUES('%s');", url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }
    return EXIT_SUCCESS;
}

/**
 * Insert the tags corresponding to a given page acquired by the tag_extractor function
 * @param connection the MySQL connection parameter
 * @param id the id corresponding to the page
 * @param tags the tags to be inserted
 * @return
 */
int db_add_tags(MYSQL *connection, int id, char *tags) {
    char query[100 * BUFSIZ];

    sprintf(query, "UPDATE Links "
            "SET tags='%s' "
            "WHERE id='%d';", tags, id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }
    return EXIT_SUCCESS;
}

/**
 * Insert the title corresponding to a given page acquired by the extract_title function
 * @param connection the MySQL connection parameter
 * @param id the id corresponding to the page
 * @param tags the tags to be inserted
 * @return
 */
int db_insert_title(MYSQL *connection, const char *title, int id) {
    char query[BUFSIZ];

    sprintf(query, "UPDATE Links"
            " SET title='%s' "
            " WHERE id='%d';", title, id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }
    return EXIT_SUCCESS;
}

int db_insert_link_map(MYSQL *connection, int from, int to) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Link_Maps (from_id, to_id) "
            " VALUES ('%d', '%d') "
            " ON DUPLICATE KEY UPDATE times = times + 1;", from, to);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }
    return EXIT_SUCCESS;
}

int db_fetch_link_id(MYSQL *connection, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "SELECT id FROM Links WHERE link='%s' ORDER BY id DESC LIMIT 1;", url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }

    int link_id = -1;
    MYSQL_RES *result = mysql_store_result(connection);
    if (mysql_num_rows(result) == 1) {
        MYSQL_ROW link_row = mysql_fetch_row(result);
        if (link_row[0]) {
            link_id = atoi(link_row[0]);
        }
    }
    mysql_free_result(result);

    return link_id;
}

int db_insert_unique_link(MYSQL *connection, int id, const char *url) {
    char query[BUFSIZ];

    sprintf(query, "INSERT INTO Links (link, occurence) "
            " VALUES ('%s', 1) "
            " ON DUPLICATE KEY UPDATE occurence = occurence + 1;", url);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    } else {
        int link_id = db_fetch_link_id(connection, url);
        if (link_id > id) {
            db_insert_link_map(connection, id, link_id);
        }
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
    }
    return EXIT_SUCCESS;
}

int db_fetch_next_id(MYSQL *connection, int id) {
    char query[BUFSIZ];

    sprintf(query, "SELECT id FROM Links WHERE id > '%d' ORDER BY id ASC LIMIT 1;", id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }

    int new_id = -1;

    MYSQL_RES *result = mysql_store_result(connection);
    if (result && mysql_num_rows(result) == 1) {
        MYSQL_ROW link_row = mysql_fetch_row(result);
        if (link_row[0]) {
            new_id = atoi(link_row[0]);
        }
    }
    if (result) mysql_free_result(result);

    return new_id;
}


int db_fetch_link(MYSQL *connection, int id, char **link) {
    char query[BUFSIZ];

    sprintf(query, "SELECT link FROM Links WHERE id = '%d' LIMIT 1;", id);

    if (mysql_query(connection, query)) {
        db_debug(connection);
    }

    MYSQL_RES *result = mysql_store_result(connection);
    if (result && mysql_num_rows(result) == 1) {
        MYSQL_ROW link_row = mysql_fetch_row(result);
        if (link_row[0]) {
            strcpy(*link, link_row[0]);
        }
    } else {
        *link = NULL;
    }
    if (result) mysql_free_result(result);

    return EXIT_SUCCESS;
}

MYSQL_RES *db_search(MYSQL *connection, const char *search) {
    char query[BUFSIZ];
    sprintf(query, "SELECT occurence, title, link, CONCAT(LEFT(tags, 200), IF(LENGTH(tags)>200, \"...\", \"\"))"
            " FROM Links\n"
            "  WHERE MATCH(tags) AGAINST ('%s')\n"
            "    ORDER BY (MATCH(tags) AGAINST ('%s')) * occurence DESC;", search, search);

    if (mysql_query(connection, query)) {
        db_debug(connection);
        notify_error("Unable to insert into database.\n");
    }

    MYSQL_RES *result = mysql_store_result(connection);
    return result;
}


#endif //WEBSITE_DOWNLOADER_DATABASE_H
