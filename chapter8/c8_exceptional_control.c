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

//回收进程wait 和 waitpid
#include<sys/types.h>
#include<sys/wait.h>
等待集合：pid=-1,等待集合是父进程的所有子进程
          pid>0,等待集合是pid一个单独的子进程
第二个参数会被放入导致返回的子进程的状态信息
第三个参数options，默认0表示挂起调用进程，直到他等待集合中的一个子进程中止
pid_t wait(int *statusp);
pid_t waitpid(pid_t pid, int *statusp, int options);
调用wait(&status)相当于调用waitpid(-1,&status,0)



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
pid_t pid;
int status;
for(i=0;i<N;i++)
   if((pid=Fork()) == 0)
       exit(100+i);

while((pid = waitpid(-1,&status,0))>0){
   if(WIFEXITED(status))
}

