Protocol Analyzer
---

## Application Protocol

### SSH

SSH는Secure SHell으로서 TCP 위의 application이다.

#### interaction

먼저 TCP connection을 연다. 그 이후 SSHv2 프로토콜을 이용하여 서버와 클라이언트가 각자 identify한다. 이어 로그인을 위하여 어떤 key algorithm을 사용할 지와 그 키를 교환한다. 이렇게 로그인이 되고 나면 서버와의 통신을 암호화하여 주고받는다.

#### header detail

별도의 헤더가 따로 존재하지 않는다. 위와 같이 패킷 안에 SSH 메시지가 들어있다. 암호화 방식을 교환하는 phase에서는 어떤 암호화 방식인지 패킷에 내용이 들어있긴 하나, 이것을 헤더라고 보기는 어렵다.

### MOSH

MOSH는 MObile SHell로서, SSH와 유사한 application이다.

#### interaction

앞서 언급한 SSH와 같은 과정을 거쳐 로그인한다. 이후 UDP로 전환하여 terminal emulator를 통해 통신한다.

#### header detail

MOSH 또한 별도의 헤더가 없다.

### HTTP

`http://snu.ac.kr`에 접속하는 과정이다.

#### interaction

HTTP get으로 요청한 뒤, HTTP 200으로 응답하고 있는 모습이다. (31, 34번 패킷)

#### header detail

위에서 `GET / HTTP/1.1`이 HTTP의 request 헤더이다. `HTTP/1.1 200 OK`가 HTTP의 response 헤더이다.

### DNS

#### interaction

A 및 AAAA 도메인 조회를 하여 이에 대해서 응답하고 있다.

#### header detail

DNS 헤더는 트랜잭션 식별자(2byte), 플래그(2byte), 카운트(8byte) 총 12byte로 구성된다.

* 트랜잭션 ID: 위에서는 0x8e09의 값을 지니고 있다. 질의-응답을 연결하는 역할을 한다.
* 플래그: 해당 메시지가 질의인지 아닌지 등 다양한 정보를 담고 있다. 캡쳐에 정확한 내용에 대해 담겨 있다.
* 카운트
  * Questions: 질의의 수
  * Answer RRs: 응답 Resource Record의 수
  * Authority RRs: 네임서버 수
  * Additional RRs: 추가 정보의 수

### WHOIS

#### interaction

클라이언트의 Query에 대해서 Answer을 보내고 있다.

#### header detail

WHOIS에서는 별다른 header가 없다.
다만 질의와 응답 패킷에 아래와 같은 Query, Answer 항목이 있다.


## ICMP

#### header detail

ICMP는 타입, 코드, check sum, rest of header 총 네 부분으로 이루어져 있다.


### ping

#### interaction

위와 같이 ping request를 보내면 서버에서 reply가 날아온다.


### traceroute

#### interaction

hop이 1일 때, 2일 때 등을 순차적으로 TTL을 늘려 가며 확인한다. 이 때 TTL이 작아 목적지에 도착하지 못하고 ICMP를 통해 TTL unreachable을 받는데, 이것으로 해당 router까지 얼마의 시간이 소요됐는지를 알 수 있다. 또한 쓰지 않는 UDP 포트에 메시지를 보내서,목적지에 도착한 것은 Port unreachable을 통해 알 수 있다.

요청은 UDP를 통해서 보내게 되며, 응답은 이 UDP 내용을 그대로 포워딩한 ICMP로 받게 된다. ICMP 메시지의 Data를 확인하면 보낸 UDP 패킷의 내용과 동일함을 확인할 수 있다.
