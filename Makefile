pzp: pzp.c
	$(CC) -Wall -O2 -static $^ -lncursesw -ltinfo -o $@

install: pzp
	install pzp /usr/local/bin/pzp
