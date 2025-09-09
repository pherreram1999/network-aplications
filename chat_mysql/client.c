#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT 8002
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};

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


    while (true) {
        printf("mensaje: ");
        scanf("%s", message);
        printf("\n");

        // Enviar mensaje
        send(sock, message, strlen(message), 0);
        printf("Mensaje enviado al servidor.\n");

        // Recibir respuesta
        read(sock, buffer, BUFFER_SIZE);
        printf("Respuesta del servidor: %s\n", buffer);
    }

    // Cerrar socket
    close(sock);

    return 0;
}
