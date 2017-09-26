#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <memory>

#include "common.h"

#define TOTAL_USER 4

class User {
public:
    string ID;
    bool is_online = false;
    int socket = 0;
    queue<pair<char*, int>> msg_queue;

    User(string id);
    void send(char* msg, int len);
    void send_all();
    bool login(int soc);
    bool logout();
};

class Message {
public:
    string type;
    string user;
    string content;

    Message(string input);
    void reply(bool err, User &u1, User &u2);
    void reply(bool err, User &u1);
};

class Group {
public:
    vector<User> member;

    bool has(User &u);
    string user_list();
    bool create(User &s, User &r);
    bool join(User &u);
    bool left(User &u);
    void send(char* msg, int len, User &u);
};

User* get_user(string id);
int clnt_connection(int clnt_sock);


mutex mtx_group;
map<string, unique_ptr<mutex>> mtx_user;
vector<User> user_db;
Group group;

int main(int argc, char** argv) {
    int serv_sock;
    int clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    for (int i = 0; i < 5; i++) {
        string id = string(1, 'A'+i);
        user_db.emplace_back(id);
        mtx_user.emplace(id, make_unique<mutex>());
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

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

User* get_user(string id) {
    for (int i = 0; i < user_db.size(); i++) {
        if (user_db[i].ID == id) {
            return &user_db[i];
        }
    }
    error_handling("get_user() - user not exist");
    return NULL;
}

int clnt_connection(int clnt_sock) {
    int str_len = 0;
    char* buffer;
    User *user = NULL;

    while (read_int(clnt_sock, str_len) != 0) {
        string raw_msg;
        if (read_string(clnt_sock, raw_msg, str_len) == str_len) {
            Message m = Message(raw_msg);

            // execute the request and handling exception
            if (user == NULL && m.type == TYPE_LOGIN) {
                user = get_user(m.content);
                if (user)
                    m.reply(user->login(clnt_sock), *user);
                else {
                    string response = TYPE_LOGIN_FAIL;
                    buffer = new char[response.size() + 4];
                    packetize(buffer, response);
                    send(clnt_sock, (void*)buffer, (response.size() + 4), 0);
                    delete buffer;
                }
            } else if (user == NULL) {
                string response = ERR_NOT_USER;
                buffer = new char[response.size() + 4];
                packetize(buffer, response);
                send(clnt_sock, (void*)buffer, (response.size() + 4), 0);
                delete buffer;
            } else if (m.type == TYPE_LOGOUT) {
                m.reply(user->logout(), *user);
            } else if (m.type == TYPE_READ_QUEUED_MSG) {
                user->send_all();
            } else if (m.type == TYPE_CREATE_GROUP) {
                User *receiver = get_user(m.user);
                if (receiver)
                    m.reply(group.create(*user, *receiver), *user, *receiver);
                else
                    m.reply(false, *user);
            } else if (m.type == TYPE_INVITE) {
                User *receiver = get_user(m.user);
                if (receiver)
                    m.reply(true, *user, *receiver);
                else
                    m.reply(false, *user);
            } else if (m.type == TYPE_LEFT_GROUP) {
                m.reply(group.left(*user), *user);
            } else if (m.type == TYPE_JOIN_GROUP) {
                m.reply(group.join(*user), *user);
            } else if (m.type == TYPE_MSG) {
                m.reply(group.has(*user), *user);
            } else {
                error_handling("clnt_connection() - unknown type error");
                m.reply(false, *user);
                continue;
            }

        } else {
            error_handling("read_string() - too short message");
            break;
        }
    }

    close(clnt_sock);
    return 0;
}

User::User(string id) {
    ID = id;
}

void User::send(char* msg, int len) {
    if (is_online) {
        ::send(socket, (void*)msg, len, 0);
    } else {
        lock_guard<mutex> loc(*mtx_user[ID]);
        char* buffer = new char[len];
        strncpy(buffer, msg, len);

        msg_queue.emplace(buffer, len);
    }
}

void User::send_all() {
    if (is_online) {
        lock_guard<mutex> loc(*mtx_user[ID]);
        while (!msg_queue.empty()) {
            char* tosend = msg_queue.front().first;
            ::send(socket, (void*)tosend, msg_queue.front().second, 0);
            delete tosend;
            msg_queue.pop();
        }
    } else {
        error_handling("User::send_all() - it is not online");
    }
}

bool User::login(int soc) {
    if (!is_online) {
        lock_guard<mutex> loc(*mtx_user[ID]);
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
        lock_guard<mutex> loc(*mtx_user[ID]);
        is_online = false;
        socket = -1;
        return true;
    } else {
        error_handling("User::logout() - it is already logged out");
        return false;
    }
}

Message::Message(string input) {
    type = input.substr(0, TYPE_SIZE);
    if (type == TYPE_LOGIN) {
        content = input.substr(TYPE_SIZE, ID_SIZE);
    } else if (type == TYPE_CREATE_GROUP) {
        user = input.substr(TYPE_SIZE, ID_SIZE);
        content = input.substr(TYPE_SIZE+ID_SIZE, input.size());
    } else if (type == TYPE_INVITE) {
        user = input.substr(TYPE_SIZE, ID_SIZE);
    } else if (type == TYPE_MSG) {
        content = input.substr(TYPE_SIZE, input.size());
    } else {
        error_handling("Message() - wrong type");
    }
}

void Message::reply(bool success, User &u, User &u2) {
    char* buffer;
    string response;
    int len;
    if (type == TYPE_CREATE_GROUP) {
        if (success) {
            response = TYPE_NEW_GROUP + group.user_list();
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            u2.send(buffer, len);
            delete buffer;

            response = TYPE_CREATE_GROUP + u.ID + content;
            len = response.size() + 4;
            buffer = new char[len];
            u2.send(buffer, len);
            delete buffer;
        } else {
            response = TYPE_ERR + ERR_CREATE_GROUP;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    }
}

void Message::reply(bool success, User &u) {
    char* buffer;
    string response;
    int len;
    if (type == TYPE_LOGIN) {
        if (success) {
            response = TYPE_LOGIN_SUCC;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    } else if (type == TYPE_INVITE) {
        if (success) {
            response = TYPE_NEW_GROUP + group.user_list();
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;

            response = TYPE_INVITE+user;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            group.send(buffer, len, u);
            delete buffer;
        } else {
            response = ERR_INV;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    } else if (type == TYPE_LEFT_GROUP) {
        if (success) {
            response = TYPE_LEFT_GROUP + u.ID;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            group.send(buffer, len, u);
            delete buffer;
        } else {
            response = TYPE_ERR + ERR_LEFT_GROUP;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    } else if (type == TYPE_JOIN_GROUP) {
        if (success) {
            response = TYPE_NEW_GROUP + group.user_list();
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;

            response = TYPE_JOIN_GROUP + u.ID;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            group.send(buffer, len, u);
            delete buffer;
        } else {
            response = TYPE_ERR + ERR_JOIN_GROUP;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    } else if (type == TYPE_MSG) {
        if (success) {
            response = TYPE_MSG + content;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            group.send(buffer, len, u);
            delete buffer;
        } else {
            response = TYPE_ERR + ERR_MSG;
            len = response.size() + 4;
            buffer = new char[len];
            packetize(buffer, response);
            u.send(buffer, len);
            delete buffer;
        }
    } else {
        response = TYPE_ERR + ERR_TYPE;
        len = response.size() + 4;
        buffer = new char[len];
        packetize(buffer, response);
        u.send(buffer, len);
        delete buffer;
    }
}


bool Group::has(User &u){
    lock_guard<mutex> lock(mtx_group);
    for (auto i : group.member)
        if (i.ID == u.ID) return true;
    return false;
}

string Group::user_list() {
    lock_guard<mutex> lock(mtx_group);
    string ret;
    for (auto i : group.member)
        ret += i.ID;
    return ret;
}

bool Group::create(User &s, User &r) {
    lock_guard<mutex> lock(mtx_group);
    if (group.member.empty()) {
        group.member.push_back(s);
        group.member.push_back(r);
        return true;
    } else {
        return false;
    }
}

bool Group::join(User &u) {
    lock_guard<mutex> lock(mtx_group);
    for (int i = 0; i < group.member.size(); i++) {
        if (group.member[i].ID == u.ID) {
            error_handling("Group::join() - user already exists");
            return false;
        }
    }
    group.member.push_back(u);
    return true;
}

bool Group::left(User &u) {
    lock_guard<mutex> lock(mtx_group);
    for (int i = 0; i < group.member.size(); i++) {
        if (group.member[i].ID == u.ID) {
            group.member.erase(group.member.begin() + i);
            return true;
        }
    }
    error_handling("Group::join() - user doesn't exist");
    return false;
}

void Group::send(char* msg, int len, User &user) {
    lock_guard<mutex> lock(mtx_group);
    for (int i = 0; i < group.member.size(); i++) {
        if (group.member[i].ID != user.ID) {
            group.member[i].send(msg, len);
        }
    }
}
