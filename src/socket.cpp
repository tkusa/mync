#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket.h"
#include "log.h"

using namespace std;

string Socket::getRhost()
{
    return (string) this->rhost;
}

int Socket::getRport()
{
    return this->rport;
}

bool Socket::isOpen()
{
    return this->open;
}

void Socket::setUdp(bool val)
{
    this->udp = val;
}

bool Socket::isUdp()
{
    return this->udp;
}

int Socket::create(char *ip_addr, int port)
{
    Log log;
    this->sock = 0;
    struct sockaddr_in address;
    this->rhost = ip_addr;
    this->rport = port;

    if (udp) {
        this->sock = socket(AF_INET, SOCK_DGRAM, 0);
    } else {
        this->sock = socket(AF_INET, SOCK_STREAM, 0);
    }
    
    if (this->sock < 0) {
        cerr << "[-] Socket creation error" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_addr, &address.sin_addr) <= 0) {
        cerr << "[-] Invalid address / Address not supoprted" << endl;
        return -1;
    }

    if (connect(this->sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "[-] Connection failed" << endl;
        return -1;
    }
    
    this->open = true;
    return this->sock;
}

int Socket::serve(int port)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    if (this->udp) {
        // UDP socket
        this->connection_fd = socket(AF_INET, SOCK_DGRAM, 0);
    } else {
        // TCP socket
        this->connection_fd = socket(AF_INET, SOCK_STREAM, 0);
    }
    if (this->connection_fd < 0) {
        cerr << "[-] Socket creation error" << endl;
        return -1;
    }

    if (setsockopt(this->connection_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        cerr << "[-] setsockopt error" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(this->connection_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "[-] bind error" << endl;
        return -1;
    }
    
    if (!this->udp) {
        if (listen(this->connection_fd, 3) < 0) {
            cerr << "[-] listen error" << endl;
            return -1;
        }
        if ((this->sock = accept(this->connection_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            cerr << "[-] accept error" << endl;
            return -1;
        }
    } else {
        char* test[8196];
        struct sockaddr_in from_addr;
        int len = sizeof(from_addr);
        this->sock = this->connection_fd; 
        recvfrom(this->sock, test, sizeof(test), 0, (struct sockaddr *) &address, (socklen_t *) &addrlen);
        connect(this->sock, (struct sockaddr *) &address, addrlen);
    }

    this->rhost = inet_ntoa(address.sin_addr);
    this->rport = int(address.sin_port);

    this->open = true;
    return this->sock;
}

void Socket::deliver(char *data, int len)
{
    if (!this->open) {
        return;
    }
    int result;
    result = send(this->sock, data, len, 0);
    
    if (result <= 0) {
        this->open = false;
    }
}

int Socket::receive(char *buf, int len)
{
    if (!this->open) {
        return 0;
    }
    int result;
    result = recv(this->sock, buf, len, 0);
    if (result <= 0) {
        this->open = false;
        return 0;
    }
    return result;
}

void Socket::copy_fd(int fd)
{
    dup2 (this->sock, fd);
    close (this->sock);
    dup2 (fd, 1);
    dup2 (fd, 2);
}

int Socket::finish()
{
    close(this->sock);
    shutdown(connection_fd, SHUT_RDWR);
    this->open = false;
    return 0;
}
