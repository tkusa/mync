#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <thread>
#include <netdb.h>
#include <arpa/inet.h>

#include "socket.h"
#include "log.h"

using namespace std;

// Print help
void help() {
    cout << "mync 0.1.0" << endl
         << "usage: mync [-Bhlnuvz] [-e executable] [-p port]" << endl
         << "            [destination] [port]" << endl
         << "options:" << endl
         << "      -B            base64 encode/decode TODO" << endl
         << "      -e executable execute program" << endl
         << "      -h            print help message" << endl
         << "      -l            listen mode" << endl
         << "      -n            avoid name resolution" << endl
         << "      -p port       custom port" << endl
         << "      -U file       upload local file TODO" << endl
         << "      -u            udp mode" << endl
         << "      -v            verbose output" << endl
         << "      -z            zero I/O" << endl
         << endl;
}

// Execute program
void execute(Socket* socket, char* exec) {
    // close socket and copy fd
    socket->copy_fd(0);
    char * p = strrchr(exec, '/');
    if (p) {
        p++;
    } else {
        p = exec;
    }
    // execute program
    execl(exec, p, NULL);
}

// Receive data from socket
void listen_connection(Socket* socket) {
    string buf;
    while (true) {
        buf = "";
        // stop if socket is closed
        if (!socket->isOpen()) {
            break;
        }
        // receive data
        buf = socket->receive();
        cout << buf;
    }
}

// Send data from socket
void client_connection(Socket* socket) {
    string buf;
    while (true) {
        buf = "";
        string tmp;
        bool eof = false;
        // read line and flg if EOF
        if (!getline(cin, tmp)) {
            eof = true;
        }
        buf += tmp + "\n";
        // stop if socket is closed
        if (!socket->isOpen()) {
            break;
        }
        // send data
        socket->deliver(buf);
        // finish if EOF
        if (eof) {
            break;
        }
    }
}

int main(int argc, char** argv) {
    Log log;
    char DEFAULT_HOST[] = "0.0.0.0";
    Socket socket;
    int x;
    // args
    extern char * optarg;
    extern int optind, optopt;
    char * host = DEFAULT_HOST;
    int port;
    char * exec;
    char * file;
    // flags
    bool b64_flg = false;
    bool exec_flg = false;
    bool listen_flg = false;
    bool name_flg = false;
    bool upload_flg = false;
    bool udp_flg = false;
    bool verbose_flg = false;
    bool zero_flg = false;

    // parse args
    while ((x = getopt(argc, argv, "Be:hlnp:U:uvz")) != EOF) {
        switch (x) {
            case 'B': // base64 encode
                b64_flg = true;
                break;
            case 'e': // execute
                exec_flg = true;
                exec = optarg;
                break;
            case 'h': // help
                help();
                exit(0);
            case 'l': // listen
                listen_flg = true;
                break;
            case 'n': // no name resolution
                name_flg = true;
                break;
            case 'p': // port
                if (optarg != NULL) {
                    port = atoi(optarg);
                }
                break;
            case 'U': // file upload
                upload_flg = true;
                file = optarg;
                break;
            case 'u': // udp
                udp_flg = true;
                break;
            case 'v': // verbose
                verbose_flg = true;
                break;
            case 'z': // zero I/O
                zero_flg = true;
                break;
        }
    }
    // log setting
    log.setVerbose(verbose_flg);
    // udp mode
    socket.setUdp(udp_flg);

    // set args
    if (argv[optind] != NULL) {
        host = argv[optind++];
    }
    if (argv[optind] != NULL) {
        port = atoi(argv[optind++]);
    }
    // name resolution
    if (!name_flg) {
        struct addrinfo hints, *info;
        memset( &hints, 0, sizeof(hints) );
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        getaddrinfo(host, NULL, &hints, &info);
        struct in_addr addr;
        if (!info) {
            log.error("[-] Host not found.");
            exit(-1);
        }
        addr.s_addr = ((struct sockaddr_in *)(info->ai_addr))->sin_addr.s_addr;
        host = inet_ntoa(addr);
    }

    if (listen_flg) {
        // listen mode
        fprintf(stderr, "[+] Listening on %s:%d...\n", host, port);
        socket.serve(port);
        if (!socket.isOpen()) {
            exit(-1);
        }
        fprintf(stderr, "[+] Connection recived from %s:%d\n", socket.getRhost().c_str(), socket.getRport());
    } else {
        // client mode
        fprintf(stderr, "[+] Connecting to %s:%d...\n", host, port);
        socket.create(host, port);
        if (!socket.isOpen()) {
            exit(-1);
        }
        if (!udp_flg) {
            log.info("[+] Connected!");
        }
    }

    // I/O
    if (!zero_flg) {
        if (exec_flg) {
            // execute program
            execute(&socket, exec);
        }
        // receive data on another thread
        thread server(listen_connection, &socket);
        // send data
        client_connection(&socket);
        // close
        server.join();
    }
    
    socket.finish();
    log.info("[+] Connection closed");
    exit(0);
}