#include <cstdlib>

#include "common.h"

bool login(int sock);
int send_message(int sock);
int recv_message(int sock);
int cmd_type(string s);
string parse_recv(string s);
string parse_send(string s);
ssize_t send_packet(int sock, string s);

string id;
string message;


int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERV_IP);
    serv_addr.sin_port = htons(SERV_PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    bool is_login = false;
    while(!is_login)
        is_login = login(sock);
    thread snd_thread(send_message, sock);
    thread rcv_thread(recv_message, sock);

    snd_thread.join();
    rcv_thread.join();

    close(sock);
    return 0;
}

int send_message(int sock) {
    string cmd;

    while (1) {
        // TODO
        // 3-1. read queued msg
        // 4. inv, cre, msg
        // 5. react inv

        cout << "< " << id << " >: ";
        getline(cin, cmd);
        int type = cmd_type(cmd);
        switch (type) {
            case (CMDTYPE_EMPTY):
                cout << "[ERROR]: 메시지를 입력하세요." << endl;
                break;
            case (CMDTYPE_ERR):
                cout << "[ERROR]: 명령어가 아닌 경우 //를 입력하여 /를 전송하세요." << endl;
                break;
            case (CMDTYPE_MSG):
                send_packet(sock, TYPE_MSG+cmd);
                break;
            default:
                send_packet(sock, parse_send(cmd));
                break;
        }
    }
}

int recv_message(int sock) {
        // TODO
        // 4. prettify
    int str_len = 0;

    while (read_int(sock, str_len) != 0) {
        string raw_msg;
        if (read_string(sock, raw_msg, str_len) == str_len) {
            cout << parse_recv(raw_msg) << endl;
        }
    }
    error_handling("recv_message() - server disconnected");
}

bool login(int sock) {
    int len = TYPE_SIZE+ID_SIZE;
    cout << "Input ID: ";
    getline(cin, id);
    string login = TYPE_LOGIN + id;
    send_packet(sock, login);

    int str_len;
    string raw_msg;
    read_int(sock, str_len);
    if (read_string(sock, raw_msg, str_len) == str_len)
        cout << parse_recv(raw_msg) << endl;
    else
        error_handling(ERR_UNKNOWN);
    return raw_msg == TYPE_LOGIN_SUCC;
}

int cmd_type(string s) {
    if (s.empty())
        return CMDTYPE_EMPTY;
    if (s.size() > 1 && s.substr(0, 2) == "//")
        return CMDTYPE_MSG;
    if (s[0] == '/') {
        if (s == CMD_READ_ALL
            || s == CMD_LOGOUT
            || s == CMD_LEFT_GROUP
            || s == CMD_ACCEPT_INVITE
            || s == CMD_DECLINE_INVITE
            || s.substr(0, CMD_INVITE.size()) == CMD_INVITE
            || s.substr(0, CMD_CREATE_GROUP.size()) == CMD_CREATE_GROUP)
            return CMDTYPE_CMD;
        return CMDTYPE_ERR;
    } else
        return CMDTYPE_MSG;
}

string parse_recv(string s) {
    return s;
}

string parse_send(string s) {
    return s;
}

ssize_t send_packet(int sock, string s) {
    if (s.size() > MSG_SIZE)
        return -1;
    int ret = send_int(sock, s.size()) + send_string(sock, s, s.size());
    return ret;
}
