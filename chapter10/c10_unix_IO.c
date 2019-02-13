/*******************************************************************************/
所有的I/O设备（磁盘，网络，终端）都被模拟成是文件
分类：普通文件(文本文件和二进制文件)
	  目录文件(包含一组链接的文件，每个链接都将一个文件名映射到一个文件)， 
	  套接字（用来与另一个进程进行跨网络通信的文件）

/*******************************************************************************/
//打开文件
打开文件：应用程序要求内核打开文件来访问I/O设备，内核从可用的描述符池中返回一个非负整数，即描述符，直到被关闭之前，对文件的所有操作都用该描述符标识，应用程序只需记住这个描述符。
linux创建的任何进程开始时就已经打开了三个文件：标准输入，标准输出和标准错误，描述符分别为0，1，2或STDIN_FILENO,STDOUT_FILENO,STDERR_FILENO
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int open(char*filename, int flags, mode_t mode);
返回：成功返回新文件描述符，错误返回-1.

int fd1=open("foo.txt",O_RDONLY,DEF_MODE)
filename即要打开的文件名
flags:指明进程将如何访问打开的这个文件，注意与文件本身的权限的区别
	  O_RDONLY:只读，进程将只读该文件，前提：该文件是可读的
	  O_WRNOLY:只写，进程将只写该文件，前提：该文件是可写的
	  O_RDER:  读写，...
	  O_CREAT: 如果文件不存在就创建一个截断的（空）文件
	  O_TRUNC:如果文件已经存在就截断它
	  O_APPEND:在每次写操作前设置文件位置到文件结尾处
mode：当创建新文件时用来设置文件的访问权限，每个进程都有一个umask，文件的权限位被设置为mode &~ umask，设置方法
	  #define DEF_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
	  #define DEF_UMASK S_IWGRP|S_IWOTH

	  umask(DEF_UMASK);
	  fd = open("foo.txt",O_CREAT|O_TRUNC|O_WRONLY,DEF_MODE);

进程通过调用open函数打开已存在的文件或创建新的文件
使用注意点：
1文件是否已经存在，如不存在需不需要创建
2创建新文件时要注意文件的读写权限

/*******************************************************************************/
//关闭已达开的文件
内核关闭文件，释放描述符到可用描述符池，当一个进程终止，内核会关闭所有其打开的文件

#include<unistd.h>
int close(int fd);
返回：成功则为0,出错则为-1

/*******************************************************************************/
//读和写文件
文件位置：既然要读写就要指出读哪个文件从文件的哪里开始读写，对于每个打开的文件，内核保持着一个文件位置k，初始为0,是从文件头起始的偏移量
读： 从文件当前位置k开始复制n>0个字节到内存，，然后将k增加到k+n。当k超过文件大小的字节数m会触发EOF
写： 从内存复制n>0个字节到一个文件，从当前文件位置k开始，然后更新k
#include<unistd.h>
ssize_t read(fd, const void*buf, size_t n);//从fd的当前文件位置复制最多n个字节到buf
返回：成功返回读得字节数，若EOF则为0,若出错则为-1,被信号中断返回-1并设置errno为EINTR
ssize_t write(int fd,const void *buf, size_t n);从内存位置buf复制最多n个字符到当前文件位
返回：若成功则为写的字节数，出错则为-1

关键：文件：fd，内存位置：buf(需由自己声明)，字节数：n
例：
#include"csapp.h"
int main(){
	char c; //这里的内存位置既然设置为char c那么，每次读写的字节数就写1,如果大了可能会出错
	while(read(STDIN_FILENO,&c,1)!=0)//STDIN_FILENO表明可以任意输入，当按下enter后才开始读一个字节到内存
	  write(STDOUT_FILENO,&c,1);//然后再向标准输出中写一个字节，然后回去再读下一个。
	exit(0);
}
输出
$ ./wr1
jhkgjgkjhg
jhkgjgkjhg

/***********************************************************************************************/
//深入理解unix I/O：不足值及其处理
read和write时需要指定读或写的字节数，但返回值可能会小于这个字节数，原因有
1：磁盘文件，EOF读到头了，否则将不会遇到不足值
2：从终端读取文本，如果打开的文件是与终端相关联的,则每次传送的是一个文本行，返回的则是实际的大小（肯定小于文本行
3：读和写网络套接字：需要读的字节数是很大的，因为缓冲区限制或者网络延迟，返回的字节数肯定也是小于全部的信息字节数的
前两者的不足值可能无所谓，毕竟该读的都读完了，但是对于第三种，一个健壮的web服务器必须反复调用read和write来处理不足值，直到所有的字节斗传送完毕。

如何处理不足值？即文件可能因为某种原因没读完就返回了，如何再接着读？
:关键是如何判断这次的read读完了：1.遇到EOF即返回值为0肯定读完了，2.读取的字节数等于设置的字节数
1.无缓冲的输入输出函数：用于将二进制数据读写到网络和从网络中读写二进制数据。
#include"csapp.h"
ssize_t rio_readn(int fd,void *usrbuf, size_t n);
ssize_t rio_written(int fd,void *usrbuf,size_t n);
当作read和write使用，优点：只有读完了文件，或者读取例要求的字节数才会停止，被信号中断了会自动重新读

ssize_t rio_read(int fd, void *usrbuf, size_t n){
	char *bufp = usrbuf;
	size_t nleft = n;
	ssize_t nread;
	while(nleft>0){
		nread = read(fd,bufp,nleft)
		if(nread <0){
			if(errno = EINTR)//被信号中断，并从信号处理程序返回，此时不会读取，所以得接着读
			  nread=0;
			else
			  return -1;//其他错误
		}
		else if(nread ==0)
		  break;
     //when nread>0或信号中断那里的nread=0
		nleft = n - nread;//所以对于网络的数据read之后总是会自动继续read
		bufp = bufp + nread;
	}
	return (n-nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}
2.带缓冲的输入输出函数：高效，便于处理
#include"csapp.h"
void rio_readinitb(rio_t *rp, int fd);//首先将文件和缓冲区关联起来
ssize_t rio_readlineb(rio_t *rp, void *usrbuf,size_t maxlen);//从内部缓冲区复制一个文本行\n结尾
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);//rio_readn的带缓冲区的版本，功能一样
返回：成功返回读的字节数，EOF则为0,若出错则为-1

//首先设置一个内存缓冲区，实际上就是在内存中模拟一个文件一个模拟文件：描述符，文件位置的指针，一块内存空间，此外还有一个参数记录未读的字节数
#define RIO_BUFSIZE 8192
typedef struct{
	int rio_fd;//描述符
	int rio_cnt;//未读取的字节数
	char *rio_bufptr;//文件位置的指针
	char rio_buf[RIO_BUFSIZE];//内存空间
}rio_t;
//rio_readinitb函数将文件和上面的内存空间关联起来
void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}
  //重点是这个rio_read函数
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf,
		   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

/* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;
    if (rp->rio_cnt < n)
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);//从内存复制到内存
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}


/*
 * rio_readnb - Robustly read n bytes (buffered)
 */
/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0)
            return -1;          /* errno set by read() */
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
 static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

// rio_readnb - Robustly read n bytes (buffered)
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0) 
            return -1;          /* errno set by read() */ 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

// rio_readlineb - Robustly read a text line (buffered)

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}

/*********************************************************************************/
目录也是一种文件，不过与前面的区别是
输入文件名——打开文件返回描述符————读写文件————关流闭文件
输入路径名——打开目录流返回指针————读取目录项————关闭目录流
//输入路径名，打开目录流，获取指针
#include<sys/types.h>
#include<dirent.h>
DIR *opendir(const char *name);//参数就是路径的字符串，返回指向该目录的指针
返回：若成功则为处理的指针，若出错则为NULL
//以指针为参数，获取下一个目录项的指针
#include<dirent.h>
struct dirent *readdir(DIR *dirp);
返回：成功返回指向下一个目录的指针，若没有更多目录项或出错则为NULL
目录项是一个结构
struct dirent{
	ino_t d_ino; //文件位置
	char d_name[256]; //文件名
}
//关闭目录流
#include<dirent.h>
int closedir(DIR *dirp);
例：
#include "csapp.h"
int main(int argc, char **argv)
{
    DIR *streamp;
    struct dirent *dep;

    /* $end readdir */
    if (argc != 2) {
        printf("usage: %s <pathname>\n", argv[0]);
        exit(1);
    }
    /* $begin readdir */
    streamp = Opendir(argv[1]);

    errno = 0;
	while((dep=readdir(streamp))!=NULL){//每次读取目录流的下一个目录项，从结果来看，肯定也有个类似于文件位置的东西，每读一次加1,因为没有重复读取一个目录项，而是读到头触发返回NULL
        printf("Found file: %s\n", dep->d_name);
    }
    if (errno != 0)
        unix_error("readdir error");

    Closedir(streamp);
    exit(0);
}
输出：
$ ./readdir /home/lm/Desktop/repository/
Found file: .git
Found file: chapter9
Found file: chapter7
Found file: .
Found file: ..
Found file: chapter10
Found file: chapter8

/***********************************************************************************/


