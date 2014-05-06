all:
	bison -d debugterm.y
	flex debugterm.l
	$(CC) debugterm.tab.c lex.yy.c -o debugterm

clean:
	rm -rf *.tab.c *.tab.h lex.yy.c debugterm
