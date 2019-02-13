#include"csapp.h"
int main(){
	int fd1,fd2;
	fd1=open("fd1.txt",O_RDWR|O_CREAT|O_APPEND,DEF_MODE);
	close(fd1);
	fd2=open("fd2.txt",O_RDWR|O_CREAT|O_APPEND,DEF_MODE);
	printf("fd2 = %d\n",fd2);
	exit(0);
}
