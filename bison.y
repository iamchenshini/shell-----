%{
#include "myGlobal.h"
#include <string.h>
#include <stdlib.h>
int yylex();
void yyerror();
void myClear();
CmdGroup cg;
%}
%union{
char * str;
};

%token <str> REINPUT REOUTPUT
%token <str>ORDER
%token <str>PARAM

%%
line  	:            {return;}
        |command  back {cmd_parse(&cg);return;}
        ;
command	:simpleCmd  pipeCmd   
        ;
simpleCmd: order args reinput reoutput 
        ;
back    :
        | '&'  {cg.isBack=1;}
;
reinput :
        | '<' PARAM {
                        int i=0;
                        int len = strlen($2);
                        cg.cmds[cg.num-1].input = (char*)malloc(sizeof(char*)*(len+1));
                        strcpy(cg.cmds[cg.num-1].input,$2);
                    }
;
reoutput :
         | '>' PARAM     {
                        int i = 0;
                        int len = strlen($2);
               
                        cg.cmds[cg.num-1].output = (char*)malloc(sizeof(char*)*(len+1));
                        strcpy(cg.cmds[cg.num-1].output , $2);
                        }
          ;
order     :ORDER	{
		                    int len = strlen($1);
                            cg.num++;
                            cg.cmds[cg.num-1].args[0] = (char*)malloc(sizeof(char*)*(len+1));
                            strcpy(cg.cmds[cg.num-1].args[0],$1);//the first args is command name
                            cg.cmds[cg.num-1].sz++;
			
                    }
            ;
pipeCmd     : 
            |'|' command     
            ;

args        :
            |param	args 
            ;
param       :PARAM {
                    int len = strlen($1);
                    int idx =  cg.cmds[cg.num-1].sz;
                    cg.cmds[cg.num-1].args[idx] = (char *)malloc(sizeof(char)*(len+1));
                    strcpy(cg.cmds[cg.num-1].args[idx],$1);
                    cg.cmds[cg.num-1].sz++;
                
                    }
            ;   

%%

void yyerror(char *errmsg)
{
    fprintf(stderr,"%s\n",errmsg);
}
int main(int argc,char **argv)
{
	int i;
	init();
    myClear();
	printf("user-sh@%s>(i am %d)",get_current_dir_name(),getpid());		
	while(1)
	{
		yyparse();
		addHistory(lineCmd);
		myClear();

		printf("user-sh@%s>(I am %d)",get_current_dir_name(),getpid());	
	}
}
void myClear()
{
    int i,j;	
	lineCmd[0]='\0';
        //printf("debuging from bison.y: cg.num is cleaned\n");
	for(i=0;i<cg.num;i++)
	{
		//free args (args are 10 pointers at most)
		for(j=0;j<10;j++)
		{
			free(cg.cmds[i].args[j]);
			cg.cmds[i].args[j]=NULL;
		}
		cg.cmds[i].sz=0;
		free(cg.cmds[i].input);
		free(cg.cmds[i].output);
		cg.cmds[i].input = NULL;
		cg.cmds[i].output = NULL;
		cg.cmds[i].isBack = 0;
	}
	cg.num=0;
    cg.isBack=0;
}
