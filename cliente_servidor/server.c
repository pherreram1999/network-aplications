/*Corro Mendoza Onasis Alejandro
  Herrera Mauricio Pedro Alonso
*/
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8001

typedef struct {
    int socket_id;
    int ID;
    char * nombre;
} Client;


Client * newClient(int socket_id) {
    Client * client = malloc(sizeof(Client));
    client->socket_id = socket_id;
    return client;
}

void * handle_connection(void *arg) {
    char buffer[BUFFER_SIZE] = {0};
    char * password = getenv("PASSWD");
    size_t bytes_received = 0;
    Client * client = (Client *) arg;
    printf("Connection established\n");
    // esperamos la primera entrada de datos
    read(client->socket_id, buffer, BUFFER_SIZE);
    // debe ser la contraseña
    printf("pass: %s\n", buffer);

    if (strncmp(buffer,password,strlen(password)) != 0) {
        char * outMessage = "contraseña no valida";
        send(client->socket_id,outMessage , strlen(outMessage), 0);
        close(client->socket_id); // cerramos conexion
        free(client);
        return NULL; // salimos de la funcion
    }

    while (true) {
        bytes_received = read(client->socket_id, buffer, BUFFER_SIZE);

        if (bytes_received == 0) {
            printf("Connection closed\n");
            close(client->socket_id);
            free(client);
            return NULL; // salimos del hilo
        }

        if (strncmp(buffer,"salir",5) == 0) {
            close(client->socket_id);
            free(client);
            return NULL;
        }

        // Enviar respuesta
        send(client->socket_id,buffer, strlen(buffer), 0);
        printf("Mensaje del cliente %s: %s\n",client->nombre,buffer);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    size_t longitud;

    pthread_t pid = 0;

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

    printf("\nlistening on %d\n",PORT);


    while (true) {
        // esuchamos conexiones entrantes
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error en accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        Client * client = newClient(client_fd);

        if (pthread_create(&pid, NULL, handle_connection, client) != 0) {
            perror("Error al crear hilo");
            free(client);
            close(client_fd);
            continue;
        }

        // una vez conetado
        pthread_detach(pid);


    }


}