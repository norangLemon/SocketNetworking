2017 Computer Network Assignment 1
---
> [Assignment description](http://incpaper.snu.ac.kr/images/5/58/2017em_Socket_HW.pdf)

## 1. Problem statement

다음과 같은 것을 구현해야 한다.

### Server

#### user & the list

* 유저의 가입 절차가 따로 없으므로 서버 프로그램에 유저 목록을 따로 저장해둔다.
* 유저의 구분은 ID로 한다.
* 유저의 로그인 여부를 따로 저장해둔다.
* 메시지 큐를 만들어서 로그아웃 중인 유저의 메시지를 순서대로 저장해둔다.

#### group

* 한 유저가 다른 유저에게 메시지를 보내면 그룹이 생성된다.
* 그룹에 속한 유저 목록을 정리해두어야 한다.
* 생성된 그룹에서 추가적으로 다른 유저를 초대하거나 나갈 수 있다.
* 그룹은 총 하나만 존재한다고 정의한다.
* 그룹에 속한 유저가 메시지를 보내면 모든 그룹의 유저에게 메시지를 전달한다.

#### undelivered messages & system announcements

* 로그인 중일 경우 시스템 알림과 메시지가 도착하는 대로 즉시 클라이언트에 전달한다.
* 로그아웃 중일 경우 메시지 큐에 저장해두었다가, 접속을 하면 쌓인 알림의 갯수를 전달한다.
* 유저가 쌓인 알림을 읽겠다고 하면 모든 메시지와 그 전송자를 일괄 전송해준다.
이 때 전송된 메시지는 큐에서 삭제된다.

#### multiplexing or multi-threading

* 서버에 최대 4개의 클라이언트가 소켓 통신을 시도하므로, 멀티플렉싱 혹은 멀티스레딩으로 이 소켓 통신을 관리해야 한다.

### Client

#### login & read message

* 프로그램을 시작하면 유저의 아이디를 입력하여 로그인한다.
* 로그인에 성공하면 읽지 않은 메시지의 총 갯수를 받는다.
* 읽지 않은 메시지를 받은 이후에 다른 조작을 할 수 있다.

#### logout

* 로그아웃을 할 수 있다.
* 로그아웃 중에 받은 메시지는 서버에 저장된다.

#### send message & make conversation

* 그룹 채팅이 시작되지 않은 경우, 대상을 지정하여 메시지를 보낼 수 있다.
* 메시지 송신자와 수신자가 포함된 그룹 채팅이 즉시 시작된다.

#### quit conversation/group

* 그룹 채팅이 시작되면 언제든지 그 그룹에서 나갈 수 있다.
* 그룹에서 나가더라도 자신이 과거에 했던 말이 사라지지는 않는다.
* 그룹에서 나간 이후에는 메시지를 받지 않는다.

#### invite user

* 그룹 채팅이 시작되면 언제든지 유저를 초대할 수 있다.
* 초대 권한은 모든 그룹 유저에게 있다.

#### invitation accept/decline

* 초대를 받게 되면 수락 혹은 거절을 할 수 있다.
* 초대 수락을 하게 된 이후 대화에 참여할 수 있다.
즉 초대 수신과 초대 수락 사이의 대화는 볼 수 없다.


## 2. The details of your implementation
(Protocol & other functions, User’s (Client) manual, …)

### Protocol

모든 패킷은 다음과 같은 구조로 이루어져 있다.

```
Packet_Size(int)|Packet_Type(String)|Receiver/Sender(String)|Content(String)
```

패킷의 종류에 따라서 Sender, Receiver, Content는 없을 수도 있다.
서버가 보내는 패킷에는 Sender만이, 유저가 보내는 패킷에는 Receiver만이 있다.
유저가 보내는 패킷은 반드시 위 구조 순대로 내용이 포함되어 있으며,
한 요소가 생략되면 잇다른 요소도 생략된다.
자세한 패킷의 구조는 아래에서 자세히 확인할 수 있다.

한편, 효율성을 위하여 Packet의 크기를 먼저 수신한 뒤 이어지는 내용을 받게 된다.
Packet의 크기는 200을 넘지 않는다.
이때 Packet_Size는 해당 값을 제외한 Packet의 전체 길이를 의미한다.

Packet_Type은 3글자로 이루어져 있다.
ID의 길이는 한 글자로만 이루어져 있다.
따라서 Content를 제외하고는 문자의 위치에 따라서 어떤 내용인지 알 수 있다.

#### User's Packet

* Log in
    * `Packet_Size|LIN`
* Log Out
    * `Packet_Size|OUT`
* Read Queued Messages
    * `Packet_Size|QUE`
* Create New Conversation
    * `Packet_Size|CRE|Receiver|Content`
* Invite
    * `Packet_Size|INV|Receiver`
* Left/Join Group
    * `Packet_Size|ACC`
    * `Packet_Size|DEC`
* Send Message
    * `Packet_Size|MSG|Receiver|Content`

#### Server's Packet

* Login Failed
    * `Packet_Size|FIN`
* Login Successed
    * `Packet_Size|SIN`
* New Conversation Created
    * `Packet_Size|CRE|Sender|Content`
    * 다른 사람이 날 초대함
* Invited
    * `Packet_Size|INV|Sender`
* Left/Join Group
    * `Packet_Size|ACC|Sender`
    * `Packet_Size|DEC|Sender`
* New Message
    * `Packet_Size|MSG|Sender|Content`


### Server Implementation

#### Multi-threading

C++11의 thread를 사용.

#### Abstraction

Group, User, Message 클래스 구성.

read_int, read_string으로 추상화하여 read를 직접 다루지 않음.

#### Logging

### Client Implementation

#### Multi-threading

C++11의 thread를 사용.

#### Abstraction

Message 클래스 구성.

read_int, read_string으로 추상화하여 read를 직접 다루지 않음.



### How to run program

## 3. Results
– Screenshots of your implementation result

## 4. Discussion


