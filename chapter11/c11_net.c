/****************************************************************************************************************************/
//ip地址的转换，十六进制转换成十进制
//网络字节顺序统一为大端顺序
ip地址存放在结构中
struct in_addr{
	uint32_t s_addr;//网络顺序
}
即对于一个IP地址
struct in_addr inaddr;//网络顺序
uint32_t addr；      //主机顺序

1.//字节顺序转换，二进制，点分十进制是字符串不存在大小端问题
//如果主机字节顺序是小端法，则unix提供了下面的函数来和网络字节顺序转换
#include<arpa/inet.h>
uint32_t htonl(uint32_t hostlong);//主机-网络.操作的是32位的整数，而不是点分十进制的字符串
uint32_t ntohl(uint32_t netlong);  //网络-主机
2.//ip地址和点分十进制之间转换、
#include<arpa/inet.h>
int inet_pton(AF_INET,const char *src,void *dst);
//将点分十进制字符串转换成网络顺序的二进制放到dst中，返回：成功为1,src非法为0,错误-1
const char *inet_ntop(AF_INET,const void *src,char *dst,socklen_t size);
//二进制网络顺序转换成点分十进制表示，返回：指向十进制字符串的指针

例：1.点分十进制字符串到网络二进制
struct in_addr inaddr;          //存放网络二进制ip地址
uint32_t addr;                  //存放主机二进制ip地址
char *buf = "128.2.194.282";    //存放点分十进制字符串
inet_pton(AF_INET,buf,&inaddr); //转换成二进制网络IP地址
addr = ntohl(inaddr.s_addr);    //转换成主机ip地址

2.主机二进制IP地址转换成点分十进制
struct in_addr inaddr;      //存放网络二进制iP地址
uint32_t addr=0x8002c2f2;   //存放主机二进制ip地址
char* buf[MAXBUF];          //存放点分十进制字符串
inaddr.s_addr = gtonl(addr);//主机顺序到网络顺序
inet_ntop(AF_INET,&inaddr,buf,MAXBUF);//转换成点分十进制
printf("%s\n",buf);
/****************************************************************************************************************************/
//套接字接口：是一组函数，用于创建网络应用
套接字地址结构， 有两种
IP socket地址结构：
struct sockaddr_in {              //共16个字节
	uint16_t sin_family;          /*16位，协议家族，总为AF_INET*/
	uint16_t sin_port;            /*16位，端口号，网络顺序*/
	struct in_addr; sin_addr;     /*32位，ip地址，网络顺序*/
	unsigned char sin_zero[8];    /*8个字节，没什么用，只是为了保持和sockaddr一样长*/
}

通用socket地址结构，只用于connect，bind和accept
struct sockaddr{         //共16字节
	uinta6_t sa_family;  /*16位，协议家族*/
	char sa_data[14];    /*14字节，ip地址和端口号*/
}

区别联系：1. 程序员操作的应该是sockaddr_in，而sockaddr是由sockaddr_in强制类型转换过去的，即sockaddr是由操作系统操作的，程序员不能操作;
2. 通用的sockaddr只用于个别函数，并且将ip地址和端口号放到了一起，在网络编程中我们都是对sockaddr_in进行操作，_in是internet缩写

1.//socket函数:创建一个套接字描述符
//套接字就是一个有相应描述符的打开的文件，类似open函数打开文件并返回一个描述符
#include<sys/types.h>
#include<sys/soket.h>
int socket(int domain,int type,int protocol);
返回：成功返回非负描述符，出错为-1
说明：socket返回唯一的ID用来确定往哪个链接上传输数据
使用：clientfd=Socket(AF_INET,SOCK_STREAM,0);
AF_INET表明使用32位ip地址，即ipv4,SOCK_STREAM表明这个套接字是链接的一个断点

2.//connect函数：将clientfd和服务器socketaddr发起连接
#include<sys/socket.h>
int connect(int clientfd,const struct sockaddr *addr,socklen_t addrlen);
返回：成功为0出错为-1
第一个参数为本地socket调用返回的clientfd，第二个参数是服务器socket地址，可见是个通用socket地址，用来和服务器建立连接，第三个参数是sockaddr_in的长度，即16字节，connect函数会阻塞直到连接成功或发生错误，如果成功便可以使用clientfd读写了，得到的链接是由套接字对刻画的即 客户端ip地址:临时端口

3.//bind函数，注意，这个与客户端没有关系：将服务器端sockfd与服务器自己的socketaddr联系起来
//服务器启动时会绑定一个众所周知的ip+port，供客户端进行绑定
#include<sys/socket.h>
int bind(int sockfd,const struct sockaddr *addr,socklen_t addrlen);
返回：成功返回0.错误返回-1

4.//listen函数，将服务器端的sockfd从主动套接字变为监听套接字listenfd，listenfd可以接收来自客户端的链连接请求
//内核认为socket产生的描述符对应于active socket，但是只有客户端是主动发起方，服务器端应当是被动实体，所以。。。
#include<sys/socket.h>
int listen(int sockfd,int backlog);
返回：成功则为0,出错则为-1
backlog参数则是TCP在开始拒绝链接请求之前，队列中要排队的未完成的来拿接请求的数量。

5.//accept函数，等待listenfd上的连接请求，在通用socket地址addr中填写客户端的套接字地址供返回信息使用，并返回一个已连接描述符connfd
#include<sys/socket.h>
int accept(int listenfd,struct sockaddr *addr,int *addrlen);
返回：成功则为非负已连接描述符connfd，若出错则为-1;

描述符：
除了在客户端和服务器调用socket函数返回的clientfd和sockfd，还出现了监听描述符listenfd以及已连接描述符connfd
listenfd和connfd区别：
监听描述符是客户端连接请求的一个端点，只创建一次，存在于服务器的整个生命周期，一个listenfd可以接受所有到达服务器的请求。
已连接描述符是客户端和服务器之间已建立起来的连接的一个端点，每次连接请求都会创建一次，它只存在于服务器为一个客户端服务的过程中。

客户端：clientfd
服务器端：sockfd->listenfd->多个connfd，区分listenfd和connfd是为了实现并发
所以最终是clientfd和connfd之间建立连接，客户端返回使用clientfd，服务器端返回connfd给应用程序，然后客户端和服务器就可以分别通过读和写clientfd和connfd来回传送数据了。

 这个描述符和前面的一样，012是留给标准文件的，其他的从3开始

小结：
int socket(int domain,int type,int protocol);
int connect(int clientfd,const struct sockaddr *addr,socklen_t addrlen);
int bind(int sockfd,const struct sockaddr *addr,socklen_t addrlen);
int listen(int sockfd,int backlog);
int accept(int listenfd,struct sockaddr *addr,int *addrlen);
1.//socket函数:创建一个套接字描述符
2.//connect函数：将clientfd和服务器socketaddr发起连接
3.//bind函数，注意，这个与客户端没有关系：将服务器端sockfd与服务器自己的socketaddr联系起来
4.//listen函数，将服务器端的sockfd从主动套接字变为监听套接字listenfd，listenfd可以接收来自客户端的链连接请求
5.//accept函数，之前必须经过bind和listen的处理，accept才能开始等待listenfd上的连接请求，在通用socket地址addr中填写客户端的套接字地址供返回信息使用，并返回一个已连接描述符connfd
//连接的端点是套接字地址，包括地址端口等

/*****************************************************************************************************************************/
1// socket网络编程—addrinfo结构体定义：
  struct addrinfo {
  int ai_flags;            /* customize behavior */
  int ai_family;           /* socket函数的第一个参数 */
  int ai_socktype;         /* socket函数的第二个参数 */
  int ai_protocol;         /* socket函数的第三个参数 */
  socklen_t ai_addrlen;    /* length in bytes of address */
  struct sockaddr *ai_addr;/* 通用套接字地址 */
  char *ai_canonname;      /* 主机名 */
  struct addrinfo *ai_next;/* next in list */ 
  };
其中主要参数：
ai_family:指定了地址族，可取值如下：
AF_INET          2            IPv4
AF_INET6        23            IPv6
AF_UNSPEC        0            协议无关

ai_socktyp:指定我套接字的类型
SOCK_STREAM        1            流
SOCK_DGRAM        2            数据报

在AF_INET通信域中套接字类型SOCK_STREAM的默认协议是TCP（传输控制协议）
在AF_INET通信域中套接字类型SOCK_DGRAM的默认协议是UDP（用户数据报协议）

2 // getaddrinfo函数将主机名(www.baidu.com)主机地址(10.3.8.217)，服务名(http)和端口号（port）的字符串表示转化成套接字地址结构
//getaddrinfo函数返回的是一个指向由结构addrinfo组成的链表的指针，这个链表最多有3个结构，原因下面。
getaddrinfo解决了把主机名和服务名转换成套接口地址结构的问题:
#include<netdb.h>
int getaddrinfo( const char *hostname, const char *service, const struct addrinfo *hints, struct addrinfo **result );
返回： 成功返回0，出错返回非0的错误代码

其中相关参数：
1.hostname:域名或ip地址串(IPv4的点分十进制串或者IPv6的16进制串)，如www.baidu.com
2.service：一个服务名（如http）或者10进制端口号数串，如果不想把主机名和service转换成地址，可以设置为NULL，但两者必须设置一个。
3.hints：是一个addrinfo结构，如果要传递这个参数，则只能设置结构的前四个字段，即标志ai_flag，地址族ai_family，套接字类型ai_socktype,协议ai_protocol,其它字段必须设置为0。，一般用memset将整个结构清零，然后有选择的设置一些字段，具体设置及其作用如下：
ai_family:getaddrinfo默认可以返回IPv4和IPv6套接字地址，ai_family设置为AF_INET会将列表限制为IPv4,AF_INET6限制为ipv6。
ai_socktype:对于host关联的每个地址，getaddrinfo函数最多返回3个addrinfo结构，每个的ai_socktype字段不同，一个是连接，一个是数据包，一个是原始套接字。设置为SOCK_STREAM将列表限制为对每个地址最多一个addrinfo结构，该结构的套接字地址可以作为连接的一个端点。
ai_flags:位掩码，用于进一步修改默认行为，常用值有：AI_ADDRCONFIG,AI_CANONNAME,AI_NUMERICSERV,AI_PASSIVE
4.result指针参数返回一个指向addrinfo结构链表的指针,该结构链表由getaddrinfo函数创建，创建时会根据前面三个参数的值自动填写每个字段，除了ai_flags。

addrinfo结构的最后有一个指向下一个结构的指针，说明有很多这种结构构成了一个链表P657,而result就指向这个链表
在客户端调用getaddrinfo之后会遍历这个链表，依次尝试每个套接字地址，直到调用socket和connect成功，建立起连接。
服务器端也会尝试遍历链表中的每个套接字地址，直到调用socket可bind成功，描述符会绑定到一个合法的套接字地址。
getaddrinfo函数创建的addrinfo结构中的字段无需再进行处理，可以直接传给套接字接口中的函数：ai_family,ai_socktype和ai_protocol可以直接传给socket,ai_addr和ai_addrlen可以直接传给connect和bind。
应用程序必须在最后释放这个由addrinfo结构构成的链表以避免内存泄漏，


3 //getnameinfo函数：和getaddrinfo相反，将一个套接字地址结构转换成相应的主机和服务名字符串。
#include<sys/socket.h>
#include<netdb.h>
int getnameinfo(const struct sockaddr *sa, socklen_t salen, 
			              char *host, size_t hostlen, 
						  char *service, size_t servlen, int flags);
返回：成功为0,出错为非0的错误代码

sa指向大小为salen字节的套接字地址结构，host指向大小为hostlen字节的缓冲区，service指向大小为servlen字节的缓冲区。
getnameinfo函数将套接字地址结构sa转换成对应的主机和服务名字符串，并将他们复制到host和service缓冲区。
#include"csapp.h"
int main(int argc,char **argv){
	struct addrinfo *p, *listp, hints;
	char buf[MAXLINE];
	int rc,flags;
	if(argc!=2){
		printf(stderr,"usage:%s<domain name>\n",argv[0]);
		exit(0);
	}
	/* get a list of addrinfo records */
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	if((rc=getaddrinfo(argv[1],NULL,&hints,&listp))!=0){
		fprintf(stderr,"getaddrinfo error: %s\n",gai_strerroe(rc));
		exit(1);
	}
	/*walk the list and display each ip address*/
	flags = NI_NUMERCHOST; /*display address string instead of domain name*/
	for(p=listp;p;p=p->ai_next){
		Getnameinfo(p->ai_addr,p->ai_addrlen,buf,MAXLINE,NULL,0,flags);
		printf("%s\n",buf);
	}
	/*clean up*/
	Freeaddrinfo(listp);
}



