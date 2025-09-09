//
// Created by sistemas on 9/9/25.
//
#include <mariadb/mysql.h>
#include <stdio.h>
#include <stdlib.h>

#define CREATE_MESSAGES_TABLE_SQL \
"CREATE TABLE IF NOT EXISTS messages(" \
"    id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY," \
"    cliente_id INT UNSIGNED," \
"    nombre VARCHAR(255)," \
"    message VARCHAR(255)" \
");"

MYSQL * openConnection(char * host,char * user, char * passwd, char * dbname) {

    MYSQL * connection = mysql_init(NULL);
    if (connection == NULL) {
        fprintf(stderr, "Error initialising mysql\n");
        exit(EXIT_FAILURE);
    }
    if (mysql_real_connect(
        connection,
        host,
        user,
        passwd,
        dbname,
        3306,
        NULL,
        0
        ) == NULL) {
        fprintf(stderr, "Error connecting to database\n");
        exit(EXIT_FAILURE);
    }

    return connection;
}


MYSQL * openPoolA() {
    return openConnection(
        getenv("P1_HOST"),
        getenv("P1_USERNAME"),
        getenv("P1_PASSWORD"),
        getenv("P1_DBNAME")
    );
}

MYSQL * openPoolB() {
    return openConnection(
        getenv("P2_HOST"),
        getenv("P2_USERNAME"),
        getenv("P2_PASSWORD"),
        getenv("P2_DBNAME")
    );
}

void createSechama(MYSQL * connection) {
    if (mysql_query(connection, CREATE_MESSAGES_TABLE_SQL)) {
        fprintf(stderr, "Error creando la conexion\n");
        exit(EXIT_FAILURE);
    }
}

