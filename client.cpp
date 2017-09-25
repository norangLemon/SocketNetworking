#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>

#define BUFSIZE 100
#define NAMESIZE 20

using namespace std;

int send_message(int sock);
int recv_message(int sock);
int error_handling(string msg);

string name(NAMESIZE, '\0');
string message(BUFSIZE, '\0');

int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in serv_addr;
    
    if (argc != 4) {
        cout << "Usage : " << argv[0] << " <IP> <port> <name>\n";
        exit(1);
    }
    
    name = argv[3];

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    thread snd_thread(send_message, sock);
    thread rcv_thread(recv_message, sock);

    snd_thread.join();
    rcv_thread.join();

    close(sock);
    return 0;
}

int send_message(int sock) {
    string str(NAMESIZE+BUFSIZE, '\0');
    while (1) {
        getline(cin, message);
        if (message == "q") {
            close(sock);
            exit(0);
        }
        str = name + " " + message;
        write(sock, str.c_str(), str.size());
    }
}

int recv_message(int sock) {
    string str(NAMESIZE+BUFSIZE, '\0');
    int str_len;
    while (1) {
        str_len = read(sock, &str[0], NAMESIZE+BUFSIZE-1);
        if (str_len == -1) return 1;
        cout << str << endl;
    }
}

int error_handling(string msg) {
    cerr << msg << endl;
    exit(1);
}
