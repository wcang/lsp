all: error copy open

error: error.c
	gcc -g -O0 -Wall -o error error.c

copy: copy.c
	gcc -g -O0 -Wall -o copy copy.c

open: open.c
	gcc -g -O0 -Wall -o open open.c

clean:
	rm -rf error copy
