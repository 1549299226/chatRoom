#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>


#define SERVER_PORT 8080
#define MAX_LISTEN  128

#define BUFFER_SIZE     128

/* 用单进程/线程 实现并发 */

int main()
{   
    /* 创建套接字 句柄 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    printf("sockfd:%d\n", sockfd);

    
    /* 将本地的IP和端口绑定 */
    struct sockaddr_in localAddress;
    bzero((void *)&localAddress, sizeof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(SERVER_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    socklen_t localAddressLen = sizeof(localAddress);
    int ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    fd_set readSet;
    /* 清空集合 */
    FD_ZERO(&readSet);
    /* 把监听的文件描述符添加到读集合中, 让内核帮忙检测 */
    FD_SET(sockfd, &readSet);

    int maxfd = sockfd;

    #if 0
    /* 设置超时 */
    struct timeval timeValue;
    bzero(&timeValue, sizeof(timeValue));
    timeValue.tv_sec = 5;
    timeValue.tv_usec = 0;
    #endif

    fd_set tmpReadSet;
    /* 清除脏数据 */
    bzero(&tmpReadSet, sizeof(tmpReadSet));
    while (1)
    {
        /* 备份读集合 */
        tmpReadSet = readSet;
        ret = select(maxfd + 1, &tmpReadSet, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select error");
            break;
        }

        /* 如果sockfd在tmpReadSet集合里面, 说明有新的连接请求到达 */
        if (FD_ISSET(sockfd, &tmpReadSet))
        {
            int acceptfd = accept(sockfd, NULL, NULL);
            if (acceptfd == -1)
            {
                perror("accpet error");
                break;
            }
            /* 将通信的句柄 放到读集合 */
            FD_SET(acceptfd, &readSet);

            /* 更新maxfd的值 */
            maxfd = maxfd < acceptfd ? acceptfd : maxfd;
        }
        
        /* 程序到这个地方: 说明可能有通信 */
        for (int idx = 0; idx <= maxfd; idx++)
        {
            if (idx != sockfd && FD_ISSET(idx, &tmpReadSet))
            {
                char buffer[BUFFER_SIZE];
                /* 清除脏数据 */
                bzero(buffer, sizeof(buffer));
                /* 程序到这里, 一定有通信(老客户) */
                int readBytes = read(idx, buffer, sizeof(buffer) - 1);
                if (readBytes < 0)
                {
                    perror("read error");
                    /* 将该通信句柄从监听的读集合中删掉 */
                    FD_CLR(idx, &readSet);
                    /* 关闭文件句柄 */
                    close(idx);
                    /* 这边要做成continue..., 让下一个已ready的fd句柄进行通信 */
                    continue;
                }
                else if (readBytes == 0)
                {
                    printf("客户端断开连接...\n");
                    /* 将该通信句柄从监听的读集合中删掉 */
                    FD_CLR(idx, &readSet);
                    /* 关闭通信句柄 */
                    close(idx);
                    /* 这边要做成continue..., 让下一个已ready的fd句柄进行通信 */
                    continue;
                }
                else
                {
                    printf("recv:%s\n", buffer);

                    for (int jdx = 0; jdx < readBytes; jdx++)
                    {
                        buffer[jdx] = toupper(buffer[jdx]);
                    }

                    /* 发回客户端 */
                    write(idx, buffer, readBytes);
                    usleep(500);
                }
            }
        }

    
    }

    /* 关闭文件描述符 */
    close(sockfd);






    return 0;
}