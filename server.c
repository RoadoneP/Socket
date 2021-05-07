#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

// 1460bytes
#define BUFFER_SIZE 66000

int main(int argc, char* argv[])
{
    // fd socket
    int listenfd = 0, connfd = 0;

    // It contains the socket address.
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    char command;
    char send_message[] = "Would you like to send the file? : test.mp4";

    int str_len;

    //Client address size
    socklen_t clnt_addr_size;

    char message[20];
    char filename[20];


    // When the port is not input
    if (argc != 2)
    {
        printf("\n Usage: %s <port> \n", argv[0]);
        return 1;
    }


    //PF=> socket, AF=> sockaddr_in   Socket creation
    if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    //reset(address)
    memset(&serv_addr, '0', sizeof(serv_addr));

    //IPv4 address system
    //호스트-네트워크(long타입: 주소), INADDR_ANY : IP주소 자동 할당
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //호스트-네트워크(short타입:포트), 명령어 입력 시 첫번째 인자가 포트 번호로 이동
    serv_addr.sin_port = htons((atoi(argv[1])));

    //Address assignment by bind
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("\n Error : bind() error\n");
        return 1;
    }

    if (listen(listenfd, 5) < 0) {
        printf("\n Error: listen() error\n");
    }

    //Address size
    clnt_addr_size = sizeof(clnt_addr);

    // Accept()
    connfd = accept(listenfd, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (connfd < 0) {
        printf("\n Error : accept() error\n");
        return 1;
    }

    // Read message
    str_len = read(connfd, message, sizeof(message));
    if (str_len < 0) {
        printf("\n Error : read() error\n");
        return 1;
    }
    printf("Client->Server : %s\n", message);
    //

    // Message send
    strcpy(message, "Hello Client!");

    printf("Sending \"%s\" to Client..\n", message);
    write(connfd, message, strlen(message));

    //--------------------------File Transfer---------------------------------
    int command_len;
    int bytes_read;
    char buff[BUFFER_SIZE] = { 0, };
    int total = 0;
    char str_file_size[15];
    FILE* checkFile;
    int file_size = 0;
    int index = 0;

    write(connfd, send_message, strlen(send_message));

    command_len = read(connfd, &command, sizeof(command));

    checkFile = fopen("test.mp4", "rb");
    if (checkFile == NULL) {
        printf("\n Error : File is Null!\n");
        return 1;
    }

    fseek(checkFile, 0, SEEK_END);
    file_size = ftell(checkFile);

    printf("Total File Size : %d\n", file_size);
    sprintf(str_file_size, "%d", file_size);  // integer -> string
    write(connfd, str_file_size, strlen(str_file_size));
    printf("Send File Size to Client!\n");
    fclose(checkFile);

    if (command_len < 0) {
        printf("\n Error : read() error\n");
        return 1;
    }

    if (command == 'y' || command == 'Y') {
        clock_t start = clock();
        FILE* fp = fopen("test.mp4", "rb");

        printf("Sending in progress\n");

        while ((bytes_read = fread(buff, sizeof(char), BUFFER_SIZE, fp)) > 0) {
            total += bytes_read;
            if (bytes_read != BUFFER_SIZE && ferror(fp)) {
                printf("\n Error : File Read error\n");
                return 1;
            }

            if (write(connfd, buff, bytes_read) < 0) {
                printf("\n Error : Send Error\n");
                return 1;
            }

            ++index;
            if (index % 100 == 0)
                printf("Progressing : %d / %d\n", total, file_size);

            memset(buff, 0, BUFFER_SIZE);
        }
        clock_t end = clock();
        printf("Send Success, %d bytes send\n", total);
        printf("Time : %lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);
        fclose(fp);
    }

    close(connfd);
    close(listenfd);


    return 0;
}