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


### Server Implementation

### Client Implementation

### How to run program

## 3. Results
– Screenshots of your implementation result
## 4. Discussion