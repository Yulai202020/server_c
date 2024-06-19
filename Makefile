C_FLAGS = -lm

all:
	gcc server.c -o server ${C_FLAGS}
	gcc client.c -o cli ${C_FLAGS}

client:
	gcc client.c -o cli ${C_FLAGS}

run_client client:
	./cli

server:
	gcc server.c -o server ${C_FLAGS}

run_server server:
	./server