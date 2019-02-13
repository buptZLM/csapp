#include"csapp.h"
int main(){
    char c; //这里的内存位置既然设置为char c那么，每次读写的字节数就写1,如果大了可能会出错
	int fd1;
	fd1=open("fd1.txt",O_RDWR|O_CREAT|O_APPEND,DEF_MODE);
    while(read(STDIN_FILENO,&c,1)!=0)//STDIN_FILENO表明可以任意输入，当按下enter后才开始读一个字节到>    内存
        write(fd1,&c,1);//然后再向标准输出中写一个字节，然后回去再读下一个。
    exit(0);
} 

