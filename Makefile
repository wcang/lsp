all: error copy open fork orphan tcp_server tcp_client

error: error.c
	gcc -g -O0 -Wall -o error error.c

copy: copy.c
	gcc -g -O0 -Wall -o copy copy.c

open: open.c
	gcc -g -O0 -Wall -o open open.c

fork: fork.c
	gcc -g -O0 -Wall -o fork fork.c

orphan: orphan.c
	gcc -g -O0 -Wall -o orphan orphan.c

tcp_server: tcp_server.c
	gcc -g -O0 -Wall -o tcp_server tcp_server.c

tcp_client: tcp_client.c
	gcc -g -O0 -Wall -o tcp_client tcp_client.c

clean:
	rm -rf error copy open fork orphan tcp_server tcp_client

