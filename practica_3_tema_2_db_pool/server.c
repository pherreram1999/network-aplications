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
#include "db.c"

#define BUFFER_SIZE 1024
#define PORT 8002

typedef struct {
    int socket_id;
    MYSQL *poolA;
    MYSQL *poolB;
} Client;


Client * newClient(int socket_id) {
    Client * client = malloc(sizeof(Client));
    client->socket_id = socket_id;
    return client;
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


    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t pid = 0;
    // nos concetamos en base de datos
    MYSQL * conn_1 = openPoolA();
    MYSQL * conn_2 = openPoolB();

    createSechama(conn_1);
    createSechama(conn_2);

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
        // una vez aceptada la conexion esperamos la nom
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