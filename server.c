#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>
#include <ctype.h>

#define SERVER_PORT 9999
#define SERVER_IP "127.0.0.1"
#define LISTEN_SIZE 128
#define BUFFER_SIZE 128

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int enableopt = 1;
    //设置端口复用
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enableopt, sizeof(enableopt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(SERVER_PORT);
    clientAddress.sin_addr.s_addr = INADDR_ANY;
    //inet_pton(AF_INET, SERVER_IP, &clientAddress.sin_addr.s_addr);
    socklen_t clientAddressLen = sizeof(clientAddress);

    ret = bind(sockfd, (struct sockaddr*)&clientAddress, clientAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }
    
    ret = listen(sockfd, LISTEN_SIZE);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    
    // socklen_t clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (acceptfd == -1)
        {
            perror("accept error");
            exit(-1);
        }
        
        ret = recv(acceptfd, recvBuffer, sizeof(recvBuffer), 0);
        if (ret == -1)
        {
            perror("recv error");
            exit(-1);
        }
        printf("%s", recvBuffer);

        for (int idx = 0; idx < strlen(recvBuffer); idx++)
        {
            sendBuffer[idx] = toupper(recvBuffer[idx]); 
        }
        
        ret = send(acceptfd, sendBuffer, sizeof(sendBuffer), 0);
        if (ret == -1)
        {
            perror("send error");
            exit(-1);
        }
        memset(recvBuffer, 0, sizeof(recvBuffer));
        memset(sendBuffer, 0, sizeof(sendBuffer));
        

    }
    
    




    close(sockfd);


    return 0;
}