#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <wait.h>

#define PORT 8080

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char const *argv[])
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pid_t pid[50];

    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    memset(address.sin_zero, '\0', sizeof address.sin_zero);


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        printf("Accepted connection\n");

        int pid_c = 0;

        if ((pid_c = fork()) == 0)
        {
            printf("Locking\n");
            pthread_mutex_lock(&lock);
            char buffer[30000] = {0};
            valread = read( new_socket , buffer, 30000);
            pthread_mutex_unlock(&lock);
            sleep(1);
            write(new_socket , hello , strlen(hello));
            printf("Replied\n");
            close(new_socket);
        }
        else
        {
            pid[i++] = pid_c;
            if (i >= 49)
            {
                i = 0;
                while (i < 50) waitpid(pid[i++], NULL, 0);
                i = 0;
            }
        }
    }
    return 0;
}