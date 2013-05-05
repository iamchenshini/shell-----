%x _PARAM_
%{
#define YYSTYPE char*
#include "myGlobal.h"
#include "bison.tab.h"

%}

%%
[a-zA-Z0-9\_\/\.]+	 {
						
			       		strcat(lineCmd,yytext);
			       		yylval = yytext;
			       		BEGIN _PARAM_;
			       		return ORDER;
			 		 }
<_PARAM_>[&<>]		{   strcat(lineCmd,yytext);return yytext[0];}

<_PARAM_>"|"		{	
						strcat(lineCmd,yytext);
			  		 	BEGIN INITIAL;
			  			return yytext[0];
					}

<_PARAM_>[-%]?[a-zA-Z0-9\_\.\/\*]+ {
				strcat(lineCmd, yytext);
				yylval = yytext;
				return PARAM;
					}
<_PARAM_>[ \t]+ {strcat(lineCmd,yytext);}
<_PARAM_>\n		{BEGIN INITIAL;yyterminate();}
<_PARAM_>.		{strcat(lineCmd,yytext);printf("输入非法符号%s\n",yytext);}
[ \t]+
.               {strcat(lineCmd,yytext);printf("输入非法符号%s\n",yytext);}
\n              {yyterminate();}
%%

int yywrap(void)

{
	return 1;
}

