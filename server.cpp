#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#define BUFSIZE 100

using namespace std;

int clnt_connection(int clnt_sock);
void send_message(string msg, int len);
void error_handling(string msg);

vector<int> clnt_socks;
mutex mutx, clnt_sock_mtx;

int main(int argc, char** argv) {
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    if (argc != 2) {
        cout << "Usage: " << argv[0] << "<port>\n";
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_addr_size = sizeof(struct sockaddr_in);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, (socklen_t *) &clnt_addr_size);

        {
            lock_guard<mutex> lock(clnt_sock_mtx);
            clnt_socks.push_back(clnt_sock);
        }
        thread t1(clnt_connection, clnt_sock);
        cout << "connected: " << clnt_sock << endl;
        t1.detach();
    }

    return 0;
}



int clnt_connection(int clnt_sock) {
    int str_len = 0;
    char message[BUFSIZE];
    int i;

    while ((str_len = read(clnt_sock, message, sizeof(message))) != 0)
        send_message(message, str_len);

    {
        lock_guard<mutex> lock(clnt_sock_mtx);
        clnt_socks.erase(clnt_socks.begin() + clnt_sock);
    }

    close(clnt_sock);
    return 0;
}
void send_message(string msg, int len) {
    int i;
    lock_guard<mutex> lock(clnt_sock_mtx);
    for (i = 0; i < clnt_socks.size(); i++) {
        write(clnt_socks[i], msg.c_str(), msg.length());
    }
}
void error_handling(string msg) {
    cerr << msg << endl;
    exit(1);
}

