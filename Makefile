pzp: main.c
	$(CC) -O2 -static main.c -lncurses -ltinfo -o $@

install: pzp
	install pzp /usr/local/bin/pzp
