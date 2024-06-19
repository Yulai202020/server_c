all:
	gcc server.c -o server
	gcc client.c -o cli

client:
	gcc client.c -o cli

run_client client:
	./cli

server:
	gcc server.c -o server

run_server server:
	./server