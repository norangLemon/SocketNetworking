#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <map>

#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MSG_SIZE 200
#define TYPE_SIZE 3
#define ID_SIZE 1
#define LEN_SIZE 3

#define TYPE_MSG                string("MSG")
#define TYPE_LOGIN              string("LIN")
#define TYPE_LOGOUT             string("OUT")
#define TYPE_LOGIN_FAIL         string("FIN")
#define TYPE_LOGIN_SUCC         string("SIN")
#define TYPE_NEW_GROUP          string("NEW")
#define TYPE_CREATE_GROUP       string("CRE")
#define TYPE_READ_QUEUED_MSG    string("QUE")
#define TYPE_INVITE             string("INV")
#define TYPE_ACCEPT_INVITE      string("ACC")
#define TYPE_DECLINE_INVITE     string("DEC")
#define TYPE_LEFT_GROUP         string("GOU")
#define TYPE_JOIN_GROUP         string("GIN")
#define TYPE_ERR                string("ERR")

#define NOTI_LOGIN              string("Login Success!")
#define NOTI_UNREAD             string("Unread Messages: ")
#define NOTI_NEW_GROUP          string("New Group | Memeber: ")
#define NOTI_INVITE             string("You Invited")
#define NOTI_JOIN               string(" Join")
#define NOTI_LEFT               string(" Left")
#define NOTI_ERR                string("[SYSTEM ERR] ")

#define ERR_ID_LEN              string("Length of ID is ")+to_string(ID_SIZE)
#define ERR_MSG_LEN             string("Cannot Send Message: Length over ")+to_string(MSG_SIZE)
#define ERR_INV                 string("Cannot Invite: You have no group or Target is already member")
#define ERR_NOT_INV             string("You Are Not Invited")
#define ERR_MSG                 string("Cannot Chat: You have no group")
#define ERR_LEFT_GROUP          string("Cannot Leave Group: You have no group")
#define ERR_JOIN_GROUP          string("Cannot Join Group")
#define ERR_CREATE_GROUP        string("Cannot Create Group: There is already group")
#define ERR_LOGIN               string("Cannot Login: You already logged in or Not proper ID")
#define ERR_TYPE                string("Unknown Message Type")
#define ERR_NOT_USER            string("Login First")
#define ERR_UNKNOWN             string("Unknown Error")

#define CMD_READ_ALL            string("/read")
#define CMD_LEFT_GROUP          string("/left")
#define CMD_ACCEPT_INVITE       string("/accept")
#define CMD_DECLINE_INVITE      string("/decline")
#define CMD_LOGOUT              string("/logout")
#define CMD_INVITE              string("/invite")
#define CMD_CREATE_GROUP        string("/send")

#define CMDTYPE_EMPTY                   0
#define CMDTYPE_ERR                     1
#define CMDTYPE_MSG                     2
#define CMDTYPE_CMD                     3
#define CMDTYPE_INVITE                  4
#define CMDTYPE_CREATE_GROUP            5
#define CMDTYPE_READ_ALL                6

#define SERV_PORT 20403
#define SERV_IP "127.0.0.1"

using namespace std;

int error_handling(string msg) {
    cerr << "[ERROR] " << msg << endl;
    exit(1);
}

int error_handling(int sock, string msg) {
    cerr << "[ERROR] " << msg << endl;
    // send some error packet
}

int logging(string msg) {
    cout << "[LOG] " << msg << endl;
}

ssize_t read_string(int sock, string &str, int length) {
    char *buffer = new char[length];
    int index = 0, received = 0;
    while (received = recv(sock, buffer+index, 1, 0) != 0) {
        index += received;
        if (index == length) {
            str = buffer;
            break;
        }
    }
    delete buffer;
    return index;
}

ssize_t read_int(int sock, int &num) {
    int received = 0, i = 0;
    char* buffer = new char[LEN_SIZE];
    while (i = recv(sock, buffer+received, 1, 0) != 0) {
        received += i;
        if (received == LEN_SIZE) {
            num = atoi(buffer);
            break;
        }
    }
    delete buffer;
    return received;
}

ssize_t send_int(int sock, int num) {
    char buffer[LEN_SIZE] = {'0', '0', '0'};
    string s = to_string(num);
    for (int i = 1; i <= s.size(); i++) {
        buffer[LEN_SIZE-i] = s[s.size()-i];
    }
    send(sock, buffer, LEN_SIZE, 0);
    return LEN_SIZE;
}

ssize_t send_string(int sock, string str, int len) {
    char *buffer = new char[len];
    strncpy(buffer, str.c_str(), len);
    int ret;
    ret = send(sock, buffer, len, 0);
    delete buffer;
    return ret;
}
