#include <iostream>

using namespace std;

#ifndef SOCKET_H
#define SOCKET_H

class Socket
{
private:
    int sock;
    int connection_fd;
    char buffer[8196] = {0};
    char * rhost;
    int rport;
    bool open = false;
    bool udp = false;
public:
    string getRhost();
    int getRport();
    bool isOpen();
    void setUdp(bool val);
    bool isUdp();
    int create(char *ip_addr, int port);
    int serve(int port);
    void deliver(string data);
    string receive();
    void copy_fd(int fd);
    int finish();
};

#endif
