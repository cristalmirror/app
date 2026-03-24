#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
int main(int argc,char *argv[]) {

    int server_fd, new_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    //Create descriptor of sockets
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket fallido");
        exit(EXIT_FAILURE);
    }

    //add the socket to port 1234
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(1234);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }

    printf("[SERVIDOR]: Esperando un conexion en el puerto 1234...\n");

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Error en accept");
        exit(EXIT_FAILURE);
    }

    int bytes_leidos;
    while ((bytes_leidos = recv(new_socket, buffer,1024, 0)) > 0) {
        printf("[CLIENTE]: %s", buffer);
        memset(buffer, 0, 1024);
    }

    // el servidor imprime
    printf("Cliente se desconector\n");

    close(new_socket);
    close(server_fd);

    return 0;
}