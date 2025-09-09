#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT 8001
#define BUFFER_SIZE 1024

typedef struct {
    int ID;
    char * nombre;
    char * password;
} Client;


Client * newClient() {
    Client * client = malloc(sizeof(Client));
    return client;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char * message = {0};
    size_t longitud, passlen;

    // Crear socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error en socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir dirección IPv4 a binario
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Dirección inválida");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor (bloqueante)
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error en connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    Client * client = newClient();
    // mandamos a un hilo diferente
    // pedimos el nombre
    printf("tu nombre: \t");
    getline(&client->nombre,&longitud,stdin);
    printf("\ntu contraseña:\t");
    getline(&client->password,&passlen,stdin);
    printf("\ntu ID: \t");
    scanf("%d",&client->ID);


    // mandamos la contraseña para validar conexioon

    send(sock, client->password, passlen, 0);

    // esperamos una respuesta del servidor

    read(sock,buffer,BUFFER_SIZE);
    printf("\nEstado de la conexion: %s",buffer);


    while (true) {
        // Recibir respuesta
        if (read(sock, buffer, BUFFER_SIZE) == 0) {
            // conexion cerrada
            printf("\nconexion cerrada");
            close(sock);
            free(client);
            exit(0);
        }
        printf("Respuesta del servidor: %s\n", buffer);

        printf("\nmensaje: \t");
        getline(&message,&longitud,stdin);

        // Enviar mensaje
        send(sock, message, strlen(message), 0);
        printf("Mensaje enviado al servidor.\n");


    }
    // Cerrar socket
    close(sock);
    return 0;
}
