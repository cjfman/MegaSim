bison:
	bison -d debugterm.y
	flex debugterm.l
	
all: bison

install: bison
	cp *.c *.h ../src/.

binary: bison
	$(CC) debugterm.tab.c lex.yy.c -o debugterm

clean:
	rm -rf *.tab.c *.tab.h lex.yy.c debugterm
