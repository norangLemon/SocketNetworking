#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#define TOTAL_USER 4

#define MSG_SIZE 200
#define TYPE_SIZE 3
#define ID_SIZE 1

#define TYPE_INVITE "INV"

#define SERV_PORT 20403

using namespace std;

&User get_user(int clnt_sock);
ssize_t read_string(int clnt_sock, string &str, int length);
ssize_t read_int(int clnt_sock, int &buffer);

int clnt_connection(int clnt_sock);
void send_message(string msg);
void error_handling(string msg);
mutex mtx_group;
map<mutex> mtx_user;

vector<User> user_list;

class User {
public:
    string ID;
    bool is_online = false;
    int socket = 0;
    Group group = NULL;
    queue<Message> msg_queue;

    User(char id);
    void send(Message &msg);
    void send();
    bool login(int soc);
    bool logout();
};

class Message {
public:
    string type;
    string sender;
    string receiver;
    string content;

    Message(string input);
    void send();
};

class Group {
public:
    vector<User> user;

    bool join(User &u);
    bool exit(User &u);
    void send(Message msg);
};

int main(int argc, char** argv) {
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(SERV_PORT));

    if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        clnt_addr_size = sizeof(struct sockaddr_in);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, (socklen_t *) &clnt_addr_size);

        thread t(clnt_connection, clnt_sock);
        cout << "connected: " << clnt_sock << endl;
        t.detach();
    }

    return 0;
}

&User get_user(int sock) {
    string id;
    if (read_string(sock, id, ID_SIZE) == ID_SIZE) {
        for (int i = 0; i < user_list.size(); i++) {
            if (user_list[i].ID == id) {
                if (user_list[i].login(sock))
                    return user_list[i];
                else
                    return NULL;
            }
        }
        error_handling("get_user() - user not exist");
    } else {
        error_handling("get_user() - cannot get id");
    }
    return NULL;
}

ssize_t read_string(int sock, string &str, int length) {
    char *buffer = new char[length];
    int index = 0, received = 0;
    while (received = read(sock, buffer + index, length) != 0) {
        index += received;
        if (index == length) {
            str = buffer;
            break;
        }
    }
    free buffer;
    return index;
}

ssize_t read_int(int sock, int &num) {
    char *buffer = new char[4];
    int index = 0, received = 0;
    while (received = read(sock, buffer + index, 4) != 0) {
        index += received;
        if (index == 4) {
            num = *((int*)buffer);
            break;
        }
    }
    free buffer;
    return index;
}

int clnt_connection(int clnt_sock) {
    int str_len = 0;
    User user = get_user(clnt_sock);

    if (user == NULL) {
            close(clnt_sock);
            exit(1);
    }

    while (read_int(clnt_sock, str_len) != 0)  {
        string raw_msg;
        if (read_string(clnt_sock, raw_msg, str_len) == str_len) {
            Message m = Message(raw_msg);
            m.send();
        } else {
            error_handling("read_string(): too short message");
            break;
        }
    }

    close(clnt_sock);
    return 0;
}

void send_message(string msg) {

    lock_guard<mutex> lock(clnt_sock_mtx);
    for (int i = 0; i < clnt_socks.size(); i++) {
        write(clnt_socks[i], msg.c_str(), msg.length());
    }
}

void error_handling(string msg) {
    cerr << msg << endl;
    exit(1);
}

User::User(string id) {
    ID = id;
    mtx_user[id] = mutex();
}

void User::send(Message &msg) {
    if (is_online)
        write(socket, msg.content, msg.content.length());
    else {
        lock_guard<mutex> loc(mtx_user[ID]);
        msg_queue.push(msg);
    }
}

void User::send() {
    if (is_online) {
        lock_guard<mutex> loc(mtx_user[ID]);
        while (!msg_queue.empty()) {
            string tosend = msg_queue.front().content;
            write(socket, tosend, tosend.length());
            msg_queue.pop();
        }
    } else {
        error_handling("User::send() - it is not online");
    }
}

bool User::login(int soc) {
    if (!is_online) {
        lock_guard<mutex> loc(mtx_user[ID]);
        is_online = true;
        socket = soc;
        return true;
    } else {
        error_handling("User::login() - it is already logged in");
        return false;
    }
}

bool User::logout() {
    if (is_online) {
        lock_guard<mutex> loc(mtx_user[ID]);
        is_online = false;
        socket = -1;
        return true;
    } else {
        error_handling("User::logout() - it is already logged out");
        return false;
    }
}

Message::Message(string input) {
    // TODO: clarify protocol and define packet structure
    type = input.substr();
    sender = input.substr();
    receiver = input.substr();
    content = input.substr();
}

void Message::send() {
    // TODO: clarify protocol and call User::send() or Group::send()
    if (type == TYPE_INVITE) {}
}

bool Group::join(User &u) {
    lock_guard<mutex> mtx_group;
    for (int i = 0; i < user.size(); i++) {
        if (user[i].ID == u.ID) {
            error_handling("Group::join() - user already exists");
            return false;
        }
    }
    user.push_back(u);
    return true;
}

bool Group::exit(User &u) {
    lock_guard<mutex> mtx_group;
    for (int i = 0; i < user.size(); i++) {
        if (user[i].ID == u.ID) {
            user.erase(user.begin() + i);
            return true;
        }
    }
    error_handling("Group::join() - user doesn't exist");
    return false;
}

void Group::send(Message msg) {
    lock_guard<mutex> mtx_group;
    for (int i = 0; i < user.size(); i++) {
        if (user[i].ID != msg.sender) {
            user[i].send(msg);
        }
    }
}


