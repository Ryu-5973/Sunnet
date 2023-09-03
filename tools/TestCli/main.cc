#include <iostream>
#include <thread>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFFER 1400

void HandleTcpCreate(char* ip, int port) {
    int clientFd;
    struct sockaddr_in serverAddr;
    char buf[MAXBUFFER] = {};
    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    std::thread t([&buf, clientFd](){
        while(true) {
            bzero(buf, sizeof(buf));
            std::cin >> buf;
            write(clientFd, buf, strlen(buf));
        }
    });
    while(true) {
        bzero(buf, sizeof(buf));
        ssize_t len = 0;
        len = read(clientFd, buf, MAXBUFFER);
        if (len >= 0) {
            std::cout << buf << std::endl;
        }
    }
    close(clientFd);
}

void HandleUdpCreate(char* ip, int port) {
    int clientFd;
    struct sockaddr_in serverAddr;
    char buf[MAXBUFFER];
    clientFd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    while(true) {
        bzero(buf, sizeof(buf));
        std::cin >> buf;
        write(clientFd, buf, strlen(buf));
    }
    close(clientFd);
}

// ./TestCli [ip] [port] [protocol]  
int main(int argc, char* argv[]) {
    if(argc < 3) {
        std::cout << "[Usage] " << "./cli.sh ip port [protocol] " << std::endl;
        return -1;
    }

    int protocolType = 0;
    if(argc > 3) {
        protocolType = atoi(argv[3]);
    }

    int port = atoi(argv[2]);
    if(protocolType == 0) {     // tcp
        HandleTcpCreate(argv[1], port);
    }else if(protocolType == 1) {   // udp
        HandleUdpCreate(argv[1], port);
    }

    return 0;
}