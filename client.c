#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 66000

int main(int argc, char* argv[])
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    int str_len;
    char message[20] = "Hello Server!";
    char filename[20];

    char command;

    // If not exist IP and Port num
    if (argc != 3)
    {
        printf("\n Usage: %s <ip of server> <port> \n", argv[0]);
        return 1;
    }


    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // 0?

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));


    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("\nSending \"%s\" to Client..\n", message);
    write(sockfd, message, strlen(message));

    str_len = read(sockfd, message, strlen(message));
    if (str_len < 0) {
        printf("\n Error: read() error");
        return 1;
    }

    printf("Server->Client : %s\n", message);
    //---------------------------------File Transfer-------------------------------
    int file_size;
    char read_message[50];
    int bytes_read;
    char buff[BUFFER_SIZE] = { 0, };
    int total = 0;
    char str_file_size[15];
    int index = 0;

    read(sockfd, read_message, sizeof(read_message));
    printf("\n%s [Y/n]\n", read_message);

    scanf("%c", &command);
    write(sockfd, &command, sizeof(command));

    read(sockfd, str_file_size, 15);
    file_size = atoi(str_file_size);

    if (command == 'y' || command == 'Y') {
        clock_t start = clock();
        FILE* fp = fopen("rece.mp4", "wb");
        if (fp == NULL)
        {
            printf("\n Error : File is Null!\n");
            return 1;
        }

        printf("Download in progress\n");

        while ((bytes_read = read(sockfd, buff, BUFFER_SIZE)) > 0) {
            total += bytes_read;
            if (bytes_read < 0) {
                printf("Error : Receiving Error");
                return 1;
            }

            if (fwrite(buff, sizeof(char), bytes_read, fp) < 0) {
                printf("Error : File Write Error");
                return 1;
            }

            ++index;
            if (index % 100 == 0) {
                printf("Progressing : %d / %d \n", total, file_size);
            }

            memset(buff, 0, BUFFER_SIZE);
        }
        clock_t end = clock();

        printf("Receive Success, %d bytes received.\n", total);
        printf("Time : %lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);
        fclose(fp);
    }

    close(sockfd);

    return 0;
}