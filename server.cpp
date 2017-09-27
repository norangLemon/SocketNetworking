#include <vector>
#include <queue>
#include <algorithm>

#include "common.h"

#define TOTAL_USER 4

class User {
public:
    string ID;
    bool is_online = false;
    int socket = 0;
    queue<string> msg_queue;
    bool is_invited = false;

    User(string id);
    void send(string msg);
    void send_all();
    bool login(int soc);
    bool logout();
    void answer_invite();
    void invite_error();
    void invited();
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
    void send(string msg, User &u);
};

User* get_user(string id);
int clnt_connection(int clnt_sock);
ssize_t send_packet(int sock, string s);

mutex mtx_group;
map<string, unique_ptr<mutex>> mtx_user;
map<int, unique_ptr<mutex>> mtx_send;
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

    cout << "Server Started!" << endl;
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

        // make mutex(for send msg) per socket to avoid packet to be mixed
        mtx_send.emplace(clnt_sock, make_unique<mutex>());

        thread t(clnt_connection, clnt_sock);
        t.detach();
        cout << "connected: " << clnt_sock << endl;
    }

    return 0;
}

User* get_user(string id) {
    for (int i = 0; i < user_db.size(); i++) {
        if (user_db[i].ID == id) {
            return &user_db[i];
        }
    }
    logging("get_user() - user not exist");
    return NULL;
}

int clnt_connection(int clnt_sock) {
    int str_len = 0;
    User *user = NULL;

    while (read_int(clnt_sock, str_len) != 0) {
        string raw_msg;
        if (read_string(clnt_sock, raw_msg, str_len) == str_len) {
            Message m = Message(raw_msg);
            logging(m.type);
            // execute the request and handling exception
            if (user == NULL && m.type == TYPE_LOGIN) {
                user = get_user(m.content);
                if (user) {
                    bool suc = user->login(clnt_sock);
                    if (!suc) {
                        send_packet(clnt_sock, TYPE_ERR + ERR_LOGIN);
                        user = NULL;
                    }
                    m.reply(suc, *user);
                } else {
                    string response = TYPE_LOGIN_FAIL;
                    send_packet(clnt_sock, response);
                }
            } else if (user == NULL) {
                string response = ERR_NOT_USER;
                send_packet(clnt_sock, response);
            } else if (m.type == TYPE_LOGOUT) {
                m.reply(user->logout(), *user);
                close(clnt_sock);
                return 0;
            } else if (m.type == TYPE_READ_QUEUED_MSG) {
                user->send_all();
            } else if (m.type == TYPE_CREATE_GROUP) {
                User *receiver = get_user(m.user);
                if (receiver)
                    m.reply(group.create(*user, *receiver), *user, *receiver);
                else
                    m.reply(false, *user);
            } else if (m.type == TYPE_INVITE) {
                if (!group.has(*user))
                    m.reply(false, *user);
                User *receiver = get_user(m.user);
                if (receiver && !group.has(*receiver)) {
                    receiver->invited();
                    m.reply(true, *receiver);
                } else
                    m.reply(false, *user);
            } else if (m.type == TYPE_LEFT_GROUP) {
                m.reply(group.left(*user), *user);
            } else if (m.type == TYPE_ACCEPT_INVITE) {
                if (user->is_invited) {
                    m.reply(group.join(*user), *user);
                    user->answer_invite();
                } else
                    user->invite_error();
            } else if (m.type == TYPE_DECLINE_INVITE) {
                if (user->is_invited)
                    user->answer_invite();
                else
                    user->invite_error();
            } else if (m.type == TYPE_MSG) {
                m.reply(group.has(*user), *user);
            } else {
                error_handling(clnt_sock, "clnt_connection() - unknown type error");
                break;
            }
        } else {
            error_handling(clnt_sock, "read_string() - too short message");
            break;
        }
    }
    if (user) user->logout();
    close(clnt_sock);
    return 0;
}

ssize_t send_packet(int sock, string s) {
    lock_guard<mutex> loc(*mtx_send[sock]);
    if (s.size() > MSG_SIZE)
        return -1;
    int ret = send_int(sock, s.size()) + send_string(sock, s, s.size());
    return ret;
}

User::User(string id) {
    ID = id;
}

void User::send(string msg) {
    if (is_online) {
        send_packet(socket, msg);
    } else {
        lock_guard<mutex> loc(*mtx_user[ID]);
        msg_queue.emplace(msg);
    }
}

void User::send_all() {
    if (is_online) {
        lock_guard<mutex> loc(*mtx_user[ID]);
        while (!msg_queue.empty()) {
            send_packet(socket, msg_queue.front());
            msg_queue.pop();
        }
    } else {
        logging("User::send_all() - it is not online");
    }
}

bool User::login(int soc) {
    if (!is_online) {
        lock_guard<mutex> loc(*mtx_user[ID]);
        is_online = true;
        socket = soc;
        return true;
    } else {
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
        logging("User::logout() - it is already logged out");
        return false;
    }
}

void User::answer_invite() {
    lock_guard<mutex> loc(*mtx_user[ID]);
    is_invited = false;
}

void User::invite_error() {
    send(ERR_NOT_INV);
}

void User::invited() {
    lock_guard<mutex> loc(*mtx_user[ID]);
    is_invited = true;
}

Message::Message(string input) {
    type = input.substr(0, TYPE_SIZE);
    cout << "Message(): type is " << type << endl;
    if (type == TYPE_LOGIN) {
        content = input.substr(TYPE_SIZE, ID_SIZE);
    } else if (type == TYPE_CREATE_GROUP) {
        user = input.substr(TYPE_SIZE, ID_SIZE);
        content = input.substr(TYPE_SIZE+ID_SIZE, input.size());
    } else if (type == TYPE_INVITE) {
        user = input.substr(TYPE_SIZE, ID_SIZE);
    } else if (type == TYPE_MSG) {
        content = input.substr(TYPE_SIZE, input.size());
    }
}

void Message::reply(bool success, User &u, User &u2) {
    string response;
    int len;
    if (type == TYPE_CREATE_GROUP) {
        if (success) {
            response = TYPE_NEW_GROUP + group.user_list();
            u.send(response);
            u2.send(response);

            response = TYPE_CREATE_GROUP + u.ID + content;
            u2.send(response);
        } else {
            response = TYPE_ERR + ERR_CREATE_GROUP;
            u.send(response);
        }
    }
}

void Message::reply(bool success, User &u) {
    string response;
    int len;
    if (type == TYPE_LOGIN) {
        if (success) {
            response = TYPE_LOGIN_SUCC + to_string(u.msg_queue.size());
            u.send(response);
        }
    } else if (type == TYPE_INVITE) {
        if (success) {
            response = TYPE_INVITE;
            u.send(response);
        } else {
            response = TYPE_ERR + ERR_INV;
            u.send(response);
        }
    } else if (type == TYPE_LEFT_GROUP) {
        if (success) {
            response = TYPE_LEFT_GROUP + u.ID;
            u.send(response);
            group.send(response, u);
        } else {
            response = TYPE_ERR + ERR_LEFT_GROUP;
            u.send(response);
        }
    } else if (type == TYPE_JOIN_GROUP) {
        if (success) {
            response = TYPE_NEW_GROUP + group.user_list();
            u.send(response);

            response = TYPE_JOIN_GROUP + u.ID;
            group.send(response, u);
        } else {
            response = TYPE_ERR + ERR_JOIN_GROUP;
            u.send(response);
        }
    } else if (type == TYPE_MSG) {
        if (success) {
            response = TYPE_MSG + u.ID + content;
            group.send(response, u);
        } else {
            response = TYPE_ERR + ERR_MSG;
            u.send(response);
        }
    } else {
        response = TYPE_ERR + ERR_TYPE;
        u.send(response);
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
        ret += i.ID + ",";
    if (ret.empty()) return "[]";
    ret.pop_back();
    return "[" + ret + "]";
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
            logging("Group::join() - user already exists");
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
    logging("Group::join() - user doesn't exist");
    return false;
}

void Group::send(string msg, User &user) {
    lock_guard<mutex> lock(mtx_group);
    for (int i = 0; i < group.member.size(); i++) {
        if (group.member[i].ID != user.ID) {
            group.member[i].send(msg);
        }
    }
}
