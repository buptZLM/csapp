//进程控制 getpid 返回调用进程的pid，getppid返回父进程的pid
#include<sys/types.h>
#include<unistd.h>
pid_t getpid(void);
pid_t getppid(void);

//中止进程的方式1 收到一个默认行为是中止进程的信号 2 从主程序返回，3 调用exit函数
#include<stdlib.h>
void exit(int status);

//创建进程
#include<sys/types.h>
#include<unistd.h>
pid_t fork(void);
子进程中返回0,父进程中返回子进程ID，错误返回-1

//回收进程wait 和 waitpid
等待集合：pid=-1,等待集合是父进程的所有子进程
          pid>0,等待集合是pid一个单独的子进程
第二个参数会被放入导致返回的子进程的状态信息
第三个参数options，默认0表示挂起调用进程，直到他等待集合中的一个子进程中止
#include<sys/types.h>
#include<sys/wait.h>
pid_t waitpid(pid_t pid, int *statusp, int options);
pid_t wait(int *statusp);
调用wait(&status)相当于调用waitpid(-1,&status,0)
返回：成功为子进程PID，返回0仅在option设置为WNOHANG时并且等待集合中没有子进程中止时
如果调用进程没有子进程，则返回-1，并设置errno为ECHILD，如果waipid被一个信号中断则返回-1并设置errno为EINTR

//让进程休眠 sleep和pause
#include<unistd.h>
unsigned int sleep(unsigned int secs);
让调用进程暂时挂起一段时间，如果请求的时间到则返回0,如果被信号中断则返回剩余的秒数
#include<unistd.h>
int pause(void);
让调用函数休眠直到收到一个信号

//加载并运行程序execve
#include<unistd.h>
int execve(const char*filename,const char *argv[],const char*envp[]);

/*********************************************************************************************************/
发送信号：Unix提供的所有的发送信号的机制都是基于进程组这个概念的，方法：通过更新某个进程的上下文中的某个状态来发送信号

//每个进程只属于一个进程组，有一个进程组ID
#include<unistd.h>
pid_t getpgrp(void);
返回当前进程的进程组ID

//改变进程组
#include<unistd.h>
int setpgid(pid_t pid, pid_t pgid);
将pid的进程组改为pgid，前面是0则为当前进程的PID，后面是0则以前面的pid作为进程组ID，
例如当前PID=1231，则setpgid(0,0)表示把当前进程加入到进程组1231中

1.//用/bin/kill程序发送信号
linux> /bin/kill -9 15213   发送信号9给进程15213
linux> /bin/kill -9 -15213  发送信号9(SIGKILL)给进程组15213中的每个进程

2.//从键盘发送信号
前台作业后台作业，前台进程组后台进程组
作业job：表示对一条命令行求值创建的一个或多个进程，shell为每个job创建一个进程组，即一个job就是一个进程组包括多个进程
任何时刻只有一个前台进程和0或多个后台进程
linux> ls | sort  创建了一个由两个进程组成的前台作业，这两个进程是由linux管道连接起来的，每个进程分别运行各自的程序
ctrl+c  内核发送SIGINT到前台作业（前台进程组）的每个进程，默认终止
ctrl+z  内核发送SIGTSTP到前台作业（前台进程组）的每个进程，默认挂起

3.//用kill函数发送信号
#include<sys/types.h>
#include<signal.h>
int kill(pid_t pid,int sig);
返回：成功返回0错误返回-1
pid>0 发送信号号码sig给进程pid
pid=0 发送信号号码sig给调用进程所在进程组中的每个进程
pid<0 发送信号号码sig给进程组|pid|中的每个进程

4.//alarm函数：在secs秒后发送信号SIGALRM给调用进程
#include<unistd.h>
unsigned int alarm(unsigned int secs);
返回：前一次的闹钟剩余秒数，若之前没有设定闹钟则返回0

/********************************************************************************/

//接受信号：信号被发送到上下文中，而上下文是从用户模式进入内核模式时保存在内核栈中的进程运行所需的的信息，因此接受信号只能发生在从内核栈中恢复进程的时刻：从内核模式切换会用户模式，包括从系统调用中返回或是完成了一次上下文切换

pending位向量中维护者待处理信号的集合
blocked位向量中维护者被阻塞的信号集合

返回用户模式时，内核会检查未被阻塞的待处理信号的集合，执行信号触发的行为

//信号处理程序

每个信号都有默认行为
主要包括四种：进程终止，进程中止并转储内存，进程停止（挂起）直到被SIGCONT信号重启，进程忽略该信号。
#include<signal.h>
sighandler_t signal(int signum,sighanfler_t handler);
返回:成功返回指向前次处理程序的指针，错误返回SIF_ERR(不设置errno)

第一个参数是信号号码，第二个参数是信号处理程序
handler为SIG_IGN,忽略该信号
handler为SIG_DFL,默认行为
否则handler是用户自定义的函数地址，称为信号处理程序，调用handler为捕获信号，执行信号处理程序为处理信号

/**********************************************************************************************/

阻塞和接触阻塞信号

/***********************************************************************************************/
// unix错误包装函数
void unix_error(char *msg){
        fprintf(stderr,"%s: %s\n", msg,stderror(errno));
        exit(0);
}
pid_t Fork(void){
        pid_t pid;
        if((pid = fork())<0)
                unix_error("Fork error");
        return pid;
}



//fork,waitpid
//实现按顺序回收
pid_t pid[N],retpid;
int status;
for(int i=0;i<N;i++)
   if((pid[i]=Fork()) == 0)
       exit(100+i);
while((retpid = waitpid(pid[++],&status,0))>0){ //如果是0则回收顺序可能和创建的顺序不同，按顺序回收有利于更好的控制
   if(WIFEXITED(status))
	   printf("child %d exit status=%d\n",pid,WEXITSTATUS(status));
   else
	   printf("child %d terminted abnormally\n",pid);
}
if(errno != ECHILD)
	unix_error("waitpid error");
exit(0);

//signal信号处理程序
void handler(int sig){
	return;//什么也不做直接返回
}
unsigned int snooze(unsigned int secs){
	unsigned int rc = sleep(secs);
	printf("slept for %d of %d secs.\n",secs-rc,secs);
	return rc;
}
int main(int argc,char**argv){
	if(argc!=2){
		fprintf(stderr,"usage:%s <secs>\n",argv[0]);
		exit(0);
	}
	if(signal(SIGINT,handler) == SIG_ERR)
	  unix_error("signal error\n");
	snooze(atoi(argv[1]));
	exit(0);
}



