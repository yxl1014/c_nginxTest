
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <fstream>
#include <iostream>

using namespace std;

#define MAXLINE 4096
char *caches[100][3];
int cachesize = 0;
int ipindex = 0;
char *ips[10];
int ipsize;

void initMyngnix();

int cisExist(const char *buff) {
    int i = 0;
    if (cachesize == 0) {
        i = 0;
    } else {
        for (int j = 0; j < cachesize; ++j) {
            if (!strcmp(caches[j][0], buff)) {
                return j;
            }
        }
    }
    return i;
}

/*用指定的字符串替换要查找的字符串*/
char *StringReplace(const char *string, const char *search, const char *replace, int number) {
    char *result;
    /*首先分配一个char*的内存，然后再动态分配剩下的内存*/
    result = (char *) malloc(sizeof(char) * 1);
    memset(result, 0, sizeof(char) * 1);
    /*定义一个遍历用的指针和一个寻找位置用的指针*/
    char *p = const_cast<char *>(string);
    char *pos = const_cast<char *>(string);
    char *t_result = result;
    while (*p != '\0' && number > 0) {
        /*查找该字符串*/
        pos = strstr(p, search);
        /*结果为0说明剩下的字符串中没有该字符了*/
        if (pos == 0) {
            break;
        }
        /*分配临时字符串空间*/
        result = (char *) realloc(result, sizeof(char) * (strlen(result) + (pos - p) + strlen(replace) + 1));
        /*重新指定临时指针的位置，因为realloc重新分配内存后，其内存位置可能发生变化*/
        t_result = result + strlen(result);
        while (p < pos) {
            *t_result++ = *p++;
        }
        /*将字符串结尾置零*/
        *t_result = '\0';
        /*设置临时指针，以便赋值时使用*/
        char *t_replace = const_cast<char *>(replace);
        while (*t_replace != '\0')
            *t_result++ = *t_replace++;
        /*将字符串结尾置零*/
        *t_result = '\0';
        /*设置下一次遍历时的指针（重要）。当将临时指针指向search字符串长度后的位置，这样设置不会多赋值不必要的字符串*/
        p += strlen(search);
        number--;
    }

    /*将查找剩余的字符进行处理*/
    /*重新分配内存空间，并重新设置临时指针位置*/
    result = (char *) realloc(result, sizeof(char) * (strlen(result) + strlen(p) + 1));
    t_result = result + strlen(result);
    /*将剩余字符串赋值*/
    while (*p != '\0') {
        *t_result++ = *p++;
    }
    /*将字符串结尾置零*/
    *t_result = '\0';
    return result;
}

int ctos1(char buff[], int len, char *ip) {
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
    servaddr.sin_port = htons(9999);
    //inet_pton是一个IP地址转换函数，
    // 可以在将IP地址在“点分十进制”和“二进制整数”之间转换而且，
    // inet_pton和inet_ntop这2个函数能够处理ipv4和ipv6。
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s", ip);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    char *update = StringReplace(buff, "12321", "9999", 1);
    printf("sending msg to server \n");
    memset(sendline, 0, MAXLINE);
    memcpy(sendline, update, len);
    if (send(sockfd, sendline, strlen(sendline), 0) < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    memset(recvline, 0, MAXLINE);
    int lens = recv(sockfd, recvline, MAXLINE, 0);
    recvline[lens] = '\0';
    printf("recv msg from server:\n%s\n", recvline);

    caches[cachesize][0] = (char *) malloc(strlen(buff));
    memset(caches[cachesize][0], 0, strlen(buff));
    memcpy(caches[cachesize][0], buff, strlen(buff));

    caches[cachesize][2] = (char *) malloc(strlen(recvline));
    memset(caches[cachesize][2], 0, strlen(recvline));
    memcpy(caches[cachesize][2], recvline, strlen(recvline));


    free(update);
    close(sockfd);
    return cachesize++;
}

int myNginx(char buff[], int len) {
    int cindex;
    if ((cindex = cisExist(buff)) >= 0)
        return cindex;


    int ok = ctos1(buff, len, ips[ipindex++ % ipsize]);
    return ok >= 0 ? ok : -1;
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
    char *errresult = "HTTP/1.1 400 \nContent-Type: text/plain;charset=UTF-8\nContent-Length: 13\nKeep-Alive: timeout=60\nConnection: keep-alive\n\nerror request";

    printf("======waiting for client's request======\n");
    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
            continue;
        }
        n = recv(connfd, buff, MAXLINE, 0);
        buff[n] = '\0';
        printf("recv msg from client:\n%s\n", buff);
        int index = myNginx(buff, n + 1);
        if (index >= 0)
            send(connfd, caches[index][2], strlen(caches[index][2]), 0);
        else
            send(connfd, errresult, strlen(errresult), 0);
        close(connfd);
    }
}

int main() {
    initMyngnix();
    stoc1();
    /*char *buff = "GET / HTTP/1.1\nHost: localhost:12321\nConnection: Keep-Alive\nUser-Agent: Apache-HttpClient/4.5.13 (Java/11.0.11)\nAccept-Encoding: gzip,deflate";
    printf("%s\n\n", buff);
    char *buuf= StringReplace(buff,"12321","9999",1);
    printf("%s\n\n", buuf);*/

    return 0;
}

void initMyngnix() {
    ifstream file("/home/yxl/CLionProjects/nginxTest/ips");//假设文本内容是：hello
    file.seekg(ios::beg);

    char chstr[20];
    while (1) {
        file.getline(chstr, 20);
        if (strlen(chstr) == 0)
            break;
        chstr[strlen(chstr)] = '\0';

        ips[ipsize] = (char *) malloc(strlen(chstr));
        memset(ips[ipsize], 0, strlen(chstr));
        memcpy(ips[ipsize], chstr, strlen(chstr));
        ipsize++;
    }
    file.close();
}
