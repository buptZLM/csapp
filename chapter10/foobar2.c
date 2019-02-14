/*假定磁盘文件由6个ASCII码构成，foobar*/

#include"csapp.h"
int main(){
	int fd;
	char c;
	fd = open("foobar",O_RDONLY,0);
	if(Fork()==0){
		read(fd,&c,1);
		exit(0);
	}
	wait(NULL);
	read(fd,&c,1);
	printf("c=%c\n",c);
	exit(0);
}
//输出第二个字符 o
