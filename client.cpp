#include <cstdlib>

#include "common.h"

bool login(int sock, int& unread);
int send_message(int sock);
int recv_message(int sock);
int cmd_type(string s);
string parse_recv(string s);
string parse_send(string s);
void get_invite_var(string &id);
void get_create_var(string &id, string &msg);
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

    int unread = 0, str_len;
    while(!is_login)
        is_login = login(sock, unread);

    // read all queued msg first
    while (unread) {
        string cmd;
        getline(cin, cmd);
        if (cmd != CMD_READ_ALL)
            cout << "[ERROR]: 부재중 메시지를 먼저 읽으세요." << endl;
        else {
            send_packet(sock, parse_send(cmd));
            break;
        }
    }

    int i = 1;
    while (unread >= i && read_int(sock, str_len) != 0) {
        string raw_msg;
        if (read_string(sock, raw_msg, str_len) == str_len)
            cout << "[" << i++ << "] "<< parse_recv(raw_msg) << endl;
    }

    cout << "-- End of Unread Message --" << endl;

    thread snd_thread(send_message, sock);
    thread rcv_thread(recv_message, sock);

    snd_thread.join();
    rcv_thread.join();

    close(sock);
    return 0;
}

int send_message(int sock) {
    while (1) {
        string cmd;

        getline(cin, cmd);
        int type = cmd_type(cmd);
        string id, msg;
        switch (type) {
            case (CMDTYPE_EMPTY):
                cout << "[ERROR]: 메시지를 입력하세요." << endl;
                break;
            case (CMDTYPE_ERR):
                cout << "[ERROR]: 명령어가 아닌 경우 '/'문자로 시작하는 메시지를 쓸 수 없습니다." << endl;
                break;
            case (CMDTYPE_READ_ALL):
                cout << "[ERROR]: 이미 모든 메시지를 읽었습니다." << endl;
                break;
            case (CMDTYPE_MSG):
                send_packet(sock, TYPE_MSG+cmd);
                break;
            case (CMDTYPE_INVITE):
                get_invite_var(id);
                send_packet(sock, TYPE_INVITE+id);
                break;
            case (CMDTYPE_CREATE_GROUP):
                get_create_var(id, msg);
                send_packet(sock, TYPE_CREATE_GROUP+id+msg);
                break;
            default:
                send_packet(sock, parse_send(cmd));
                break;
        }
    }
}

int recv_message(int sock) {
    int str_len;
    while (read_int(sock, str_len) != 0) {
        string raw_msg;
        if (read_string(sock, raw_msg, str_len) == str_len) {
            string out = parse_recv(raw_msg);
            cout << out << endl;
        }
    }
    error_handling("server disconnected");
}

bool login(int sock, int &unread) {
    int len = TYPE_SIZE+ID_SIZE;
    cout << "Input ID: ";
    getline(cin, id);
    string login = TYPE_LOGIN + id;
    send_packet(sock, login);

    int str_len;
    string raw_msg;
    bool ret = false;
    read_int(sock, str_len);
    if (read_string(sock, raw_msg, str_len) == str_len) {
        cout << parse_recv(raw_msg) << endl;
        ret = raw_msg.substr(0, TYPE_SIZE) == TYPE_LOGIN_SUCC;
    }
    else
        error_handling(ERR_UNKNOWN + raw_msg);

    if (ret) unread = atoi(raw_msg.substr(TYPE_SIZE, raw_msg.size()).c_str());
    return ret;
}

int cmd_type(string s) {
    if (s.empty())
        return CMDTYPE_EMPTY;
    if (s[0] == '/') {
        if (s == CMD_LOGOUT
            || s == CMD_LEFT_GROUP
            || s == CMD_ACCEPT_INVITE
            || s == CMD_DECLINE_INVITE)
            return CMDTYPE_CMD;
        else if (s == CMD_INVITE)
            return CMDTYPE_INVITE;
        else if (s == CMD_CREATE_GROUP)
            return CMDTYPE_CREATE_GROUP;
        else if (s == CMD_READ_ALL)
            return CMDTYPE_READ_ALL;
        else
            return CMDTYPE_ERR;
    } else
        return CMDTYPE_MSG;
}

string parse_recv(string s) {
    string type = s.substr(0, TYPE_SIZE);
    string ret;
    if (type == TYPE_LOGIN_FAIL) {
        ret = ERR_LOGIN;
    } else if (type == TYPE_LOGIN_SUCC) {
        ret = NOTI_LOGIN+"\n"+NOTI_UNREAD+s[TYPE_SIZE];
    } else if (type == TYPE_NEW_GROUP) {
        ret = NOTI_NEW_GROUP + s.substr(TYPE_SIZE, s.size());
    } else if (type == TYPE_INVITE) {
        ret = NOTI_INVITE;
    } else if (type == TYPE_JOIN_GROUP) {
        ret = s.substr(TYPE_SIZE, ID_SIZE)+NOTI_JOIN;
    } else if (type == TYPE_LEFT_GROUP) {
        ret = s.substr(TYPE_SIZE, ID_SIZE)+NOTI_LEFT;
    } else if (type == TYPE_MSG) {
        ret = "< " + s.substr(TYPE_SIZE, ID_SIZE) + " > " + s.substr(TYPE_SIZE+ID_SIZE, s.size());
    } else if (type == TYPE_CREATE_GROUP) {
        ret = "< " + s.substr(TYPE_SIZE, ID_SIZE) + " > " + s.substr(TYPE_SIZE+ID_SIZE, s.size());
    } else {
        return NOTI_ERR + s.substr(TYPE_SIZE, s.size());
    }
    return ret;
}

string parse_send(string s) {
    if (s == CMD_READ_ALL)
        return TYPE_READ_QUEUED_MSG;
    if (s == CMD_LOGOUT)
        return TYPE_LOGOUT;
    if (s == CMD_LEFT_GROUP)
        return TYPE_LEFT_GROUP;
    if (s == CMD_ACCEPT_INVITE)
        return TYPE_ACCEPT_INVITE;
    if (s == CMD_DECLINE_INVITE)
        return TYPE_DECLINE_INVITE;
}

void get_invite_var(string &id) {
    while (1) {
        cout << "receiver's ID: ";
        getline(cin, id);
        if (id.size() == 1)
            break;
        cout << ERR_ID_LEN << endl;
    }
}

void get_create_var(string &id, string &msg) {
    while (1) {
        cout << "receiver's ID: ";
        getline(cin, id);
        if (id.size() == 1)
            break;
        cout << ERR_ID_LEN << endl;
    }
    cout << "message to send: ";
    getline(cin, msg);
    if (msg.size() > MSG_SIZE)
        cout << ERR_MSG_LEN << endl;
}

ssize_t send_packet(int sock, string s) {
    if (s.size() > MSG_SIZE)
        return -1;
    int ret = send_int(sock, s.size()) + send_string(sock, s, s.size());
    return ret;
}
