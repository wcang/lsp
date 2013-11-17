all: error copy

error: error.c
	gcc -g -O0 -Wall -o error error.c

copy: copy.c
	gcc -g -O0 -Wall -o copy copy.c

clean:
	rm -rf error copy
