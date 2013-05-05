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
        int isBack;     // �Ƿ��̨����
        //char **args;    // �������
        char *args[10];  //at most 10 args
        char *input;    // �����ض���
        char *output;   // ����ض���
	int sz;
    } SimpleCmd;
    typedef struct CmdGroup{
	SimpleCmd cmds[10];
	int num;
	int isBack;
    } CmdGroup;

    typedef struct History {
        int start;                    //��λ��
        int end;                      //ĩλ��
        char cmds[HISTORY_LEN][100];  //��ʷ����
    } History;

    typedef struct Job {
        int pid;          //���̺�
        char cmd[100];    //������
        char state[10];   //��ҵ״̬
        struct Job *next; //��һ�ڵ�ָ��
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
