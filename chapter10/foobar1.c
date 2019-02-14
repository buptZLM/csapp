/*分析描述符表，文件表，v-node表来确定输出，并比较
 * foobar2和foobar3*/
/* foobar1 假设已经存在文件foobar.txt,并且含有6个ASCII字符foobar*/

#include"csapp.h"
int main(){
	int fd1,fd2;
	char c;
	//打开同一文件两次
	fd1=open("foobar.txt",O_RDONLY,0);
	fd2=open("foobar.txt",O_RDONLY,0);
	read(fd1,&c,1);
	read(fd2,&c,1);
	printf("c = %c\n",c);
	exit(0);
}
