user-sh: bison.tab.o myExecute.o lex.yy.o myGlobal.h
	cc -g -o user-sh bison.tab.o myExecute.o lex.yy.o
bison.tab.o: bison.tab.c myGlobal.h
	cc -c bison.tab.c
lex.yy.o:lex.yy.c bison.tab.h myGlobal.h
	cc -c lex.yy.c
lex.yy.c:lex.l 
	flex lex.l
bison.tab.c bison.tab.h:bison.y
	bison -d bison.y
myExecute.o : myExecute.c myGlobal.h
	cc -c myExecute.c
clean: 
	rm  user-sh bison.tab.o bison.tab.c myExecute.o bison.tab.h
