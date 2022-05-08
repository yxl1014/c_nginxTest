
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#define MAXLINE 4096
int i = 0;
char *caches[100][3];

int ctos1(char buff[], int len) {
    int sockfd, n;
    char recvline[MAXLINE], sendline[MAXLINE];
    struct sockaddr_in servaddr;
//Type就是socket的类型，对于AF_INET协议族而言有流套接字(SOCK_STREAM)、数据包套接字(SOCK_DGRAM)、原始套接字(SOCK_RAW)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9999 + (++i % 3));
    //inet_pton是一个IP地址转换函数，
    // 可以在将IP地址在“点分十进制”和“二进制整数”之间转换而且，
    // inet_pton和inet_ntop这2个函数能够处理ipv4和ipv6。
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for 127.0.0.1");
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("send msg to server: \n");
    memcpy(sendline, buff, len);
    if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(recvline, 0, MAXLINE);
    int lens = recv(sockfd, buff, MAXLINE, 0);
    recvline[lens] = '\0';
    close(sockfd);
    return 0;
}

int myNginx(char buff[], int len) {

    return 0;
}

int stoc1() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[MAXLINE];
    int n;


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//将主机的无符号长整形数转换成网络字节顺序。
    servaddr.sin_port = htons(12321);//将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)　

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1) {//第二个参数就是那个backlog
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("======waiting for client's request======\n");
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }
        n = recv(connfd, buff, MAXLINE, 0);
        buff[n] = '\0';
        printf("recv msg from client:\n%s\n", buff);
        int index=myNginx(buff, n + 1);
        close(connfd);
    }
}

int main() {
    stoc1();
    return 0;
}
