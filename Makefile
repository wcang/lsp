all: error copy open fork orphan tcp_server tcp_client udp_server udp_client pipe dup_prank select

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

udp_server: udp_server.c
	gcc -g -O0 -Wall -o udp_server udp_server.c

pipe: pipe.c
	gcc -g -O0 -Wall -o pipe pipe.c

dup_prank: dup_prank.c
	gcc -g -O0 -Wall -o dup_prank dup_prank.c

udp_client: udp_client.c
	gcc -g -O0 -Wall -o udp_client udp_client.c

select: select.c
	gcc -g -O0 -Wall -o select select.c

clean:
	rm -rf error copy open fork orphan tcp_server tcp_client udp_server udp_client pipe dup_prank select

