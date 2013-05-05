#ifndef _global_H
#define _global_H

#ifdef	__cplusplus
extern "C" {
#endif   
    
    #define HISTORY_LEN 10
    
    #define STOPPED "stopped"
    #define RUNNING "running"
    #define DONE    "done"

    #include <stdio.h>
    #include <stdlib.h>
    
    typedef struct SimpleCmd {
        int isBack;     // 是否后台运行
        //char **args;    // 命令及参数
        char *args[10];  //at most 10 args
        char *input;    // 输入重定向
        char *output;   // 输出重定向
	int sz;
    } SimpleCmd;
    typedef struct CmdGroup{
	SimpleCmd cmds[10];
	int num;
	int isBack;
    } CmdGroup;

    typedef struct History {
        int start;                    //首位置
        int end;                      //末位置
        char cmds[HISTORY_LEN][100];  //历史命令
    } History;

    typedef struct Job {
        int pid;          //进程号
        char cmd[100];    //命令名
        char state[10];   //作业状态
        struct Job *next; //下一节点指针
    } Job;
    pid_t killedPid;
    char lineCmd[100]; 
    void clearCmd(SimpleCmd *psc);
    void init();
    void addHistory(char *history);
    void execute();
    void myExecute();
    void cmd_cd(char * filepath);
    void cmd_history();
   // void cmd_outer(SimpleCmd *cmd);
    void cmd_parse(CmdGroup *cg);
#ifdef	__cplusplus
}
#endif

#endif	/* _global_H */
