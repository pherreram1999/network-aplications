/*Corro Mendoza Onasis Alejandro
  Herrera Mauricio Pedro Alonso


*/
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mariadb/mysql.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8002

typedef struct {
    int socket_id;
    MYSQL *conn;
} Client;


Client * newClient(int socket_id) {
    Client * client = malloc(sizeof(Client));
    client->socket_id = socket_id;
    return client;
}

MYSQL * openConnection() {
    char *host = getenv("HOST");
    char *username = getenv("USERNAME");
    char *password = getenv("PASSWORD");
    char *dbname = getenv("DBNAME");

    MYSQL * connection = mysql_init(NULL);
    if (connection == NULL) {
        fprintf(stderr, "Error initialising mysql\n");
        exit(EXIT_FAILURE);
    }
    if (mysql_real_connect(connection, host, username, password, dbname, 3306, NULL, 0) == NULL) {
        fprintf(stderr, "Error connecting to database\n");
        exit(EXIT_FAILURE);
    }

    return connection;
}


void saveMessageInDB(char * message, MYSQL * conn) {
    char *query = "INSERT INTO messages(message) VALUES (?)";
    MYSQL_STMT * stmt = mysql_stmt_init(conn);
    mysql_stmt_prepare(stmt, query, strlen(query));
    MYSQL_BIND bind[1];

    if (stmt == NULL) {
        fprintf(stderr, "Error al inicializar statement\n");
        exit(EXIT_FAILURE);
    }

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = message;
    bind[0].buffer_length = strlen(message);
    bind[0].is_null = 0;
    bind[0].length = NULL;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Error al inicializar statement\n");
        mysql_stmt_close(stmt);
        exit(EXIT_FAILURE);
    }

    // Ejecutar la consulta
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error al ejecutar statement: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        exit(EXIT_FAILURE);
    }

    printf("Usuario insertado con ID: %llu\n", mysql_stmt_insert_id(stmt));

}

void * handle_connection(void *arg) {
    char buffer[BUFFER_SIZE] = {0};
    size_t bytes_received = 0;
    Client * client = (Client *) arg;
    printf("Connection established\n");
    while (true) {
        bytes_received = read(client->socket_id, buffer, BUFFER_SIZE);

        if (bytes_received == 0) {
            printf("Connection closed\n");
            close(client->socket_id);
            free(client);
            exit(0);
        }

        if (strncmp(buffer,"salir",5) == 0) {
            close(client->socket_id);
            free(client);
            exit(0);
        }

        // Enviar respuesta
        send(client->socket_id,buffer, strlen(buffer), 0);
        printf("Mensaje del cliente %s\n",buffer);

        saveMessageInDB(buffer,client->conn);


    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t pid = 0;
    MYSQL * connection = openConnection();

    printf("Conexion hecha");

    // creamos el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error en socket");
        exit(EXIT_FAILURE);
    }

    // Configuración de la dirección del servidor
    address.sin_family = AF_INET; // familia de IPV4
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Asignar socket a dirección y puerto
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("Error en bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }


    while (true) {
        // esuchamos conexiones entrantes
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        Client * client = newClient(client_fd);
        client->conn = connection;
        // mandamos a un hilo diferente

        if (pthread_create(&pid, NULL, handle_connection, client) != 0) {
            perror("Error al crear hilo");
            free(client);
            close(client_fd);
            continue;
        }
        pthread_detach(pid);


    }


}