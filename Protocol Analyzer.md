# Protocol Analyzer


## Ethernet

### How to setup & Generate packet

* device: Mac Air
* setup: 기기에 Wireshark 2.4.2, chrome 61.0.3163.100 설치
* packet generation: 크롬 웹 서칭



### IP

![IPv4](/Users/Olenthya/Desktop/IPv4.png)

* IP layer의 패킷이다.
* Source와 Destination, 버전, TTL 등이 명시되어 있다.



### ARP request

![arp-request](/Users/Olenthya/Desktop/arp-request.png)

* 특정 IP의 MAC 주소를 알아내기 위한 요청이다.
* Target의 MAC 주소를 알아내기 위해서 Source에서 192.168.0.31의 주소를 요청하고 있다.
* 아직 MAC 주소를 알지 못하기 때문에 Target MAC이 00:00:00:00:00:00으로 설정되어 있다.



### ARP response

![arp-reply](/Users/Olenthya/Desktop/arp-reply.png)

* 해당 IP를 가진 STA가 자신의 MAC 주소를 회신한다.



## Wlan

### How to setup & Generate packet

* device: lab에서 제공한 laptop, lap의 STA들, iphone
* setup: airpcap드라이버가 설치된 laptop에 airpcap usb 연결
* packet generation: lab에서 생산되고 있는 packet들을 수집, iphone에서 웹 서칭



### beacon

![beacon](/Users/Olenthya/Desktop/beacon.png)

* 무선 네트워크의 존재를 알리기 위해서 broadcast를 한다.
* Destination은 Broadcast(ff:ff:ff:ff:ff:ff:ff)이고, Source address는 자신의 MAC 주소(0a:30:0d:38:4b:02)임을 확인할 수 있다.



### probe request

![probe_request](/Users/Olenthya/Desktop/probe_request.png)

* probe request는 STA에서 가까운 AP를 찾기 위해 요청하는 것이다.
* 특정된 AP를 찾는 것이 아니므로, Destination은 Broadcast(ff:ff:ff:ff:ff:ff:ff)이다.



### probe response

![probe_response](/Users/Olenthya/Desktop/probe_response.png)

* probe request에 대한 AP의 응답이다.
* STA(Destination)에게 공유기(Source - 0a:30:0d:38:4b:02)가 응답을 하고 있다.





##  Wlan - specified STA

### How to setup & Generate packet

* device: lab에서 제공한 laptop, iphone
* setup: airpcap드라이버가 설치된 laptop에 airpcap usb 연결
* packet generation: iphone에서 웹 서칭



### Block Ack

![block_ack](/Users/Olenthya/Desktop/block_ack.png)

* Block Ack은 MPDU마다 매번 ACK을 보내지 않고, 한 번의 ACK만을 block acknowledgement로 보냄으로써 MAC efficiency를  향상하기 위한 ACK 프레임이다.



### RTS

![rts](/Users/Olenthya/Desktop/rts.png)

* Resquest To Send는 무선 환경에서 기기가 데이터를 보낼 것임을 알리는 패킷이다.
* 해당 패킷에서는 iphone 기기(78:31:c1:d1:e5:50)에 공유기(0a:30:0d:38:4b:02)가 데이터 전송 요청을 보내고 있다.



### CTS

![cts](/Users/Olenthya/Desktop/cts.png)

* Clear To Send는 무선 환경에서 기기가 데이터를 받을 준비가 되었음을 알리는 패킷이다.
* 해당 패킷에서는 iphone 기기가 Destination(78:31:c1:d1:e5:50)임을 알리고 있다.

