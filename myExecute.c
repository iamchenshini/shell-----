#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include "myGlobal.h"
#include <locale.h>
#include <fnmatch.h>
#include <dirent.h>
History history;
int ignore = 0;
volatile sig_atomic_t goon = 0;
Job *head = NULL;
pid_t fgPid;
char *envPath[10], cmdBuff[40];
FILE *fin,*fout;


void cmd_wildCard(SimpleCmd * cmd)
{
    int i,j,k,p,q;
    int flag1,flag2;//标记有没有通配符 标记有没有路径
    char    *pattern;
    char    *filename;
    DIR     *dir;
    struct dirent    *entry;//
    int     ret;
    char    *tmpargs[20];
    i=1;
    k=1;
    flag1=0;
    flag2=0;
    while(cmd->args[i]!=NULL)
    {
        p=0;
        int length;
        length=strlen(cmd->args[i]);
        for(j=0;j<length;j++)//将所有匹配后的命令存在tmpargs[]里
        {
            if(cmd->args[i][j]=='*')//如果存在通配符
            {
                flag1=1;
            }
            if(cmd->args[i][j]=='\/')
            {
                flag2=1;
                p=j;//记录最后一个/的位置
            }
        }
	    if(flag1==1)//存在通配符
        {
            char *route;
            if(flag2==1)//提取出路径
            {
                route=(char*)malloc(sizeof(char *)*(p+1));
                strncpy(route,cmd->args[i],p);
                dir=opendir(route);
                pattern = (char*) malloc(sizeof(char *)*(strlen(cmd->args[i])+1));
                for(j=p+1,q=0;j<strlen(cmd->args[i]);j++)
                {
                    pattern[q++]=cmd->args[i][j];
                }
            }
            else
            {
                dir = opendir("./");
                pattern = (char*) malloc(sizeof(char *)*(strlen(cmd->args[i])+1));
                pattern=cmd->args[i];
            }
            
            if(dir != NULL)
            {
                while( (entry = readdir(dir)) != NULL)
                {
                    ret = fnmatch(pattern, entry->d_name, FNM_PATHNAME|FNM_PERIOD);
                    if(ret == 0)
                    {
                        tmpargs[k] = (char*) malloc(sizeof(char *)*(strlen(entry->d_name)+p+1));
                        if(flag2==1)//如果有路径  参数要加上路径
                        {
                            strcpy(tmpargs[k],route);
                            tmpargs[k][p]='/';
                            tmpargs[k][p+1]='\0';
                            strcat(tmpargs[k],entry->d_name);
                            k++;
                        }
                        else
                        {
                            strcpy(tmpargs[k++],entry->d_name);
                        }
                    }
                    else if(ret == FNM_NOMATCH)
                    {
                        continue ;
                    }
                    else
                    {
						fprintf(stderr,"error file=%s\n", entry->d_name);
                    }
                }
                closedir(dir);
            }
			
        }
        else//如果该参数不存在通配符
        {
            tmpargs[k] = (char*) malloc(sizeof(char *)*(strlen(cmd->args[i])+1));
            strcpy(tmpargs[k++],cmd->args[i]);
        }
        
		i++;
    }
    for(i=1;i<k;i++)
    {
        cmd->args[i] = (char*) malloc(sizeof(char *)*(strlen(tmpargs[i])+1));
        strcpy(cmd->args[i],tmpargs[i]);
    }
    //printf("cmd_wideCard %s\n",cmdBuff);
    if(execv(cmdBuff, cmd->args) < 0)
    {
        perror("execv failed!\n");
        exit(1);
    }
    
}
void getEnvPath(int len, char *buf)
{
    int i, j, last = 0, pathIndex = 0, temp;
    char path[40];
    
    for(i = 0, j = 0; i < len; i++){
        if(buf[i] == ':')
		{
            if(path[j-1] != '/')
			{
                path[j++] = '/';
            }
            path[j] = '\0';
            j = 0;
            
            temp = strlen(path);
            envPath[pathIndex] = (char*)malloc(sizeof(char) * (temp + 1));
            strcpy(envPath[pathIndex], path);
            
            pathIndex++;
        }
		else
		{
            path[j++] = buf[i];
        }
    }
    envPath[pathIndex] = NULL;
}
void setGoon(){
    goon = 1;
}
int exists(char *cmdFile)
{
	 int i = 0;
   	 if((cmdFile[0] == '/' || cmdFile[0] == '.') && access(cmdFile, F_OK) == 0)
	{
        	strcpy(cmdBuff, cmdFile);
       	 	return 1;
    }
	else
	{  
        while(envPath[i] != NULL)
		{ 
            strcpy(cmdBuff, envPath[i]);
            strcat(cmdBuff, cmdFile);           
           	if(access(cmdBuff, F_OK) == 0)
	   		{
                return 1;
            }            
            i++;
       	}
    }  
    return 0; 
}
void cmd_cd(char * filepath)
{
 	if(chdir(filepath)<0)
	{
		fprintf(stderr,"cd: %s wrong file path!\n",filepath);
	}
	return;
}
void cmd_history()
{
    if(history.end==-1)
	{
		fprintf(stderr,"No command has been executed!\n");
		return;
	}
        int i= history.start;
    do{
        printf("%s\n",history.cmds[i]);
            i = (i+1)%HISTORY_LEN;
	}while(i!=(history.end+1)%HISTORY_LEN);
	return ;
}	

void addHistory(char *cmd){
    if(history.end == -1){ 
        history.end = 0;
        strcpy(history.cmds[history.end], cmd);
        return;
	}
    
    history.end = (history.end + 1)%HISTORY_LEN; 
    strcpy(history.cmds[history.end], cmd); 
    
    if(history.end == history.start){ 
        history.start = (history.start + 1)%HISTORY_LEN; 
    }
}
/*Job Part*/
Job* addJob(pid_t pid){
    Job *now = NULL, *last = NULL, *job = (Job*)malloc(sizeof(Job));
    
    job->pid = pid;
    strcpy(job->cmd, lineCmd);
    strcpy(job->state, RUNNING);
    job->next = NULL;
    
    if(head == NULL){ 
        head = job;
    }else{ 
		now = head;
		while(now != NULL && now->pid < pid){
			last = now;
			now = now->next;
		}
        last->next = job;
        job->next = now;
    }
    
    return job;
}
void rmJob2(int signo)
{
    //printf("rmJob %d\n",getpid());
    //tcsetpgrp(0,getpid());
    int status;
    //printf("debug from rmJob2:i am %d,received %d\n",getpid(),signo);
    pid_t sender=0;
    //后台进程组退出 
    if((sender=waitpid(-1,&status, WNOHANG))>0)
    {
        //if(WIFSTOPPED(status))printf("debug from cmd_fg: CHILD STOPPED\n");
        //if(WIFCONTINUED(status))printf("debug from cmd_fg: CHILD CONTINUED\n");
        //if(WIFEXITED(status))printf("debug from cmd_fg: CHILD EXISTED\n");
        //printf("debug from rmJob2: sender is %d\n",sender);
        Job *now = NULL, *last = NULL;
        now = head;
        while(now != NULL && now->pid < sender){
            last = now;
            now = now->next;
        }
        
        if(now == NULL){
            return;
        }
        
        if(now == head){
            head = now->next;
        }else{
            last->next = now->next;
        }
        printf("[%d]\t%s\t\t%s\n",sender,DONE,now->cmd);
        free(now);
        if(sender==fgPid)fgPid=0;
        
    }
    //前台进程组退出 或 挂起 或 继续执行 
    else
    {
        //printf("rmJob2\n");
        if(fgPid>0)
        {
            Job *now = NULL, *last = NULL;
            now = head;
            while(now != NULL && now->pid < fgPid){
                last = now;
                now = now->next;
            }
            
            if(now == NULL){
                return;
            }
            
            if(now == head){
                head = now->next;
            }else{
                last->next = now->next;
            }
            free(now);
            fgPid=0;
           
        }
        
        
        
    }
    //重新注册信号处理函数
    signal(SIGCHLD,rmJob2);
    
}
void cmd_jobs()
{
	Job *now = NULL;
	if(head==NULL)
	{
		printf("There is no job!\n");
		return ;
	}
	printf("index\tpid\tstate\tcommand\n");
	int i;
	for(i=1,now = head;now!=NULL;now = now->next,i++)
	{
		printf("%d\t%d\t%s\t\t%s\n",i,now->pid,now->state,now->cmd);
	}
	return;
}	
void ctrl_Z(){
    Job *now = NULL;
    
    if(fgPid == 0){ 
        return;
    }
	now = head;
	while(now != NULL && now->pid != fgPid)
		now = now->next;
    
    if(now == NULL){ 
        now = addJob(fgPid);
    }
    
    strcpy(now->state, STOPPED);
    
    
    now->cmd[strlen(now->cmd)] = '&';
    now->cmd[strlen(now->cmd) + 1] = '\0';
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    //挂起进程组号为fgPid的进程组
    kill(-fgPid, SIGSTOP);
    fgPid = 0;
    signal(SIGTSTP,ctrl_Z);
}
void ctrl_C()
{
	//Job *now = NULL;
	if(fgPid == 0)
	{
		return;
	}
    //终止进程组号为fgPid的进程组
    if(kill(-fgPid,SIGINT)<0)
	{
		printf("debug: kill sigint failed");
	}
 	fgPid = 0;
    //重新注册信号捕捉函数
    signal(SIGINT,ctrl_C);
}
//讲挂起进程或者 后台进程调入前台运行
void cmd_fg(int pid){    
    Job *now = NULL; 
	int i;
    now = head;
	while(now != NULL && now->pid != pid)
		now = now->next;
    
    if(now == NULL){ 
        printf("The job whose pid is %d not exist!\n", pid);
        return;
    }
    //设置前台进程组号
    fgPid = now->pid;
    strcpy(now->state, RUNNING);
    
    signal(SIGTSTP, ctrl_Z); 
    i = strlen(now->cmd) - 1;
    while(i >= 0 && now->cmd[i] != '&')
		i--;
    now->cmd[i] = '\0';
    //让进程组继续运行
    kill(-now->pid, SIGCONT);
    //printf("%d\n",getpgid(pid));
    //kill(pid,SIGCONT);
    int status;
    if(waitpid(fgPid,&status,WUNTRACED)<0)
    {
        //printf("debug from cmd_pipe: waitpid  %d failed\n",pid[i]);
        perror("debug from cmd_pipe:");
        return;
    }
}

void cmd_bg(int pid){
    Job *now = NULL;
    
    ignore = 1;
    
	now = head;
    while(now != NULL && now->pid != pid)
		now = now->next;
    
    if(now == NULL){ 
        printf("The job whose pid is %d not exists!\n", pid);
        return;
    }
    
    strcpy(now->state, RUNNING);
    printf("[%d]\t%s\t\t%s\n", now->pid, now->state, now->cmd);
    
    kill(-now->pid, SIGCONT);
}
void justArgs(char *str)
{
	int i,j,len;
	len=strlen(str);
	
	for(i=0,j=-1;i<len;i++)
	{
		if(str[i]=='/')
			j=i;
	}
    
	if(j!=-1)
	{
		for(i=0,j++;j<len;i++,j++)
			str[i]=str[j];
	}
	str[i]='\0';
}

void release()
{
	int i;
	for(i=0;strlen(envPath[i])>0;i++)free(envPath[i]);
}
//执行一条外部命令
void cmd_outer(SimpleCmd *cmd)
{
    pid_t pid;
    int pipeIn, pipeOut;
    if(exists(cmd->args[0]))
    {
        //printf("cmd_outer %s\n",cmd->args[0]);
        //设置输入重定向
        if(cmd->input != NULL)
        {
            //fprintf(stderr,"debug from  executeCmd[input]%s \n",cmd->input);
            if((pipeIn = open(cmd->input, O_RDONLY, S_IRUSR|S_IWUSR)) == -1){
                //fprintf(stderr,"Can not open the file!\n", cmd->input);
                perror("Can not open the file!\n");
                exit(0);
                return;
            }
            //fprintf(stderr,"debug from executeCmd [input]%s \n",cmd->input);
            if(dup2(pipeIn, 0) == -1)
            {
                perror("Input redirecting failed!\n");
                exit(0);
                return;
            }
        }
        //设置重输出定向
        if(cmd->output != NULL)
        {
            //fprintf(stderr,"debug form executeCmd [output]%s \n",cmd->output);
            //perror("debug form cmd_outer [output]%s \n");
            if((pipeOut = open(cmd->output, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR)) == -1)
            {
                //fprintf(stderr,"Can not open the file!\n", cmd->output);
                perror("Can not open the file!\n");
                exit(0);
                return ;
            }
            //fprintf(stderr,"debug from executeCmd [output]%s \n",cmd->output);
            if(dup2(pipeOut, 1) == -1){
                perror("Output redirecting failed!\n");
                exit(0);
                return;
            }
        }
        justArgs(cmd->args[0]);
        //printf("cmd_outer %s \n",cmd->args[0]);
        cmd_wildCard(cmd);
        //fprintf(stderr,"debug from cmd_outer: before execv %d\n",getpid());
        if(execv(cmdBuff, cmd->args) < 0){
            perror("execv failed!\n");
            return;
        }
    }
    else
    {
        fprintf(stderr,"此命令不存在\n");
    }
}

//区分内部命令与外部命令
void cmd_route(SimpleCmd *cmd)
{
    //printf("cmd_route %s\n",cmd->args[0]);
    if(strcmp(cmd->args[0],"history")==0)
    {
        cmd_history();
        return;
    }
    if(strcmp(cmd->args[0],"jobs")==0)
    {
        cmd_jobs();
        return ;
    }
    cmd_outer(cmd);
}

void cmd_pipe(CmdGroup *cg)
{
    //printf("cmd_pipe %s\n",cg->cmds[0].args[0]);
    int pipeNum = cg->num-1;
    int fd[10][2];
    pid_t pid[10];
    int i;
    //创建管道 管道数为命令数-1
    for(i=0;i<pipeNum;i++)
    {
        if(pipe(fd[i])<0)
        {
            printf("pipe failed");
        }
    }
    //创建子进程 同时重定向标准输入输出
    for(i=0;i<cg->num;i++)
    {
        //创建子进程
        if((pid[i]=fork())<0)
        {
            perror("fork error");
            exit(1);
        }
        // 子进程
        if(pid[i]==0)
        {
            
            //第一个子进程
            if(i==0)
            {
                close(fd[i][0]);
                if(dup2(fd[i][1],STDOUT_FILENO)!=STDOUT_FILENO)
                {
                    perror("dup2 failed");
                    close(fd[i][1]);
                    exit(1);
                }
		        close(fd[i][1]);
		        //fprintf(stderr,"debug from cmd_pipe the first child %s %d\n",cg->cmds[i].args[0],getpid());
            }
            //最后一个子进程
            else if(i == cg->num-1)
            {
                close(fd[i-1][1]);
                if(dup2(fd[i-1][0],STDIN_FILENO)!=STDIN_FILENO)
                {
                    perror("dup2 failed");
                    close(fd[i-1][0]);
                    exit(1);
                }
                close(fd[i-1][0]);
            }
            else
            {
                close(fd[i-1][1]);
                if(dup2(fd[i-1][0],STDIN_FILENO)<0)
                {
                    perror("dup2 failed");
                    close(fd[i-1][0]);
                    exit(1);
                }
                close(fd[i-1][0]);
                close(fd[i][0]);
                if(dup2(fd[i][1],STDOUT_FILENO)<0)
                {
                    perror("dup2 failed");
                    close(fd[i][1]);
                    exit(1);
                }
                close(fd[i][1]);
            }
            //fprintf(stderr,"i am %d, group id %d\n",getpid(),getpgid(0));
            cmd_route(&cg->cmds[i]);
            //子进程执行完毕后退出
            exit(0);
        }
        //父进程
        else
        {
            int status ;
            //等待子进程退出
            if(waitpid(pid[i],&status,WUNTRACED)<0)
            {
                perror("debug from cmd_pipe:");
                return;
            }
            if(i!=cg->num-1)close(fd[i][1]);
        }
    }
}
void cmd_parse(CmdGroup *cg)
{
    pid_t pid;
    //exit cd fg bg 内部命令必须由shell进程亲自完成
    if(strcmp(cg->cmds[0].args[0],"exit")==0)exit(0);
    if(strcmp(cg->cmds[0].args[0],"cd")==0)
    {
        cmd_cd(cg->cmds[0].args[1]);
        return;
    }
    if(strcmp(cg->cmds[0].args[0],"fg")==0)
    {
        if(cg->cmds[0].args[1][0]!='%')
        {
            fprintf(stderr,"Format error!");
            return;
        }
        cg->cmds[0].args[1][0]=' ';
        int pid = atoi(cg->cmds[0].args[1]);
        cmd_fg(pid);
        return;
    }
    if(strcmp(cg->cmds[0].args[0],"bg")==0)
    {
        if(cg->cmds[0].args[1][0]!='%')
        {
            printf("Format error!");
            return;
        }
        cg->cmds[0].args[1][0]=' ';
        int pid = atoi(cg->cmds[0].args[1]);
        cmd_bg(pid);
        return;
    }
    //注册信号用于父子进程各自设置goon变量
    signal(SIGUSR1,setGoon);
    if((pid = fork())<0)
    {
        perror("fork failed");
    }
    if(pid==0)
    {
        //子进程创建新的进程组 让自己成为该组组长
        if(setpgid(0,getpid())<0)perror("setpid failed");
        //tcsetpgrp(0,getpid());
        //若为后台命令
        if(cg->isBack)
        {
            while(goon == 0);//等待父进程把自己加入到job队列
            goon = 0;
            //重置
            printf("[%d]\t%s\t\t%s\n", getpid(), RUNNING, cmdBuff);
            //通知父进程可以返回 接受下一条命令
            kill(getppid(), SIGUSR1);
            
        }
        //printf("PARSE %s\n",cg->cmds[0].args[0]);
        //管道命令
        if(cg->num>1)cmd_pipe(cg);
        //单条命令
        else cmd_route(&cg->cmds[0]);
        exit(-1);
    }
    else
    {
        //父进程设置子进程的进程组号
        if(setpgid(pid,pid)<0)perror("setpid failed");
        
        if(cg->isBack)
        {
            fgPid = 0;
            addJob(pid);
            //通知子进程执行命令
            kill(pid, SIGUSR1);
            while(goon == 0);
            goon = 0;
        }
        else
        {
            //设置前台进程组号
            fgPid = pid;
            //tcsetpgrp(0,pid);
            int status;
            if(waitpid(pid,&status,WUNTRACED)<0)
            {
                perror("debug from cmd_pipe:");
                return;
            }
            /*else
            {
                if(WIFSTOPPED(status))printf("CHILD STOPPED\n");
                if(WIFCONTINUED(status))printf("CHILD CONTINUED\n");
                if(WIFEXITED(status))printf("CHILD EXISTED\n");
            }*/
            
        }
        //tcsetpgrp(0,getpid());
    }
}
void init()
{
    int fd, n, len;
    char c, buf[80];
	
    if((fd = open("ysh.conf", O_RDONLY, 660)) == -1)
    {
        perror("init environment failed\n");
        exit(1);
    }
    
    history.end = -1;
    history.start = 0;
    
    len = 0;
	
    while(read(fd, &c, 1) != 0)
	{ 
        buf[len++] = c;
    }
    buf[len] = '\0';
    getEnvPath(len, buf);
    signal(SIGCHLD,rmJob2);
    /*struct sigaction action;
    action.sa_sigaction = rmJob;
    sigfillset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &action, NULL);*/
    signal(SIGTSTP, ctrl_Z);
    signal(SIGINT,ctrl_C);
}

