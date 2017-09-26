#include <iostream>
#include <string>

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

#define ERR_INV                 string("Cannot Invite")
#define ERR_MSG                 string("Don't have group")
#define ERR_LEFT_GROUP          string("Cannot Leave Group")
#define ERR_JOIN_GROUP          string("Cannot Join Group")
#define ERR_CREATE_GROUP        string("Cannot Create Group")
#define ERR_LOGIN               string("Cannot Login")
#define ERR_TYPE                string("Unknown Message Type")
#define ERR_NOT_USER            string("Login First")

#define MSG_SIZE 200
#define TYPE_SIZE 3
#define ID_SIZE 1

#define SERV_PORT 20403
#define SERV_IP "127.0.0.1"

using namespace std;

void packetize(char* buffer, string input) {
    // given buffer which size is input.size(),
    // copy the string and put the size in the head of buffer
    // to make it packetized.
    int len = input.size();
    *((int*)buffer) = len;
    for (int i = 0; i < len; i++)
        buffer[i+4] = input[i];
}

int error_handling(string msg) {
    cerr << msg << endl;
    exit(1);
}

ssize_t read_string(int sock, string &str, int length) {
    char *buffer = new char[length];
    int index = 0, received = 0;
    while (received = recv(sock, buffer + index, length, 0) != 0) {
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
    char *buffer = new char[4];
    int index = 0, received = 0;
    while (received = recv(sock, buffer + index, 4, 0) != 0) {
        index += received;
        if (index == 4) {
            num = *((int*)buffer);
            break;
        }
    }
    delete buffer;
    return index;
}
