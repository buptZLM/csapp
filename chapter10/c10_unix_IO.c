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
#include<unistd.h>
int close(int fd);
返回：成功则为0,出错则为-1



