all: error copy open fork

error: error.c
	gcc -g -O0 -Wall -o error error.c

copy: copy.c
	gcc -g -O0 -Wall -o copy copy.c

open: open.c
	gcc -g -O0 -Wall -o open open.c

fork: fork.c
	gcc -g -O0 -Wall -o fork fork.c

clean:
	rm -rf error copy open fork
