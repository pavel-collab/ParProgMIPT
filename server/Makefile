all: master server client

master:
	mpic++ master.c -o master
server:
	mpicc server.c -o server
client:
	mpicc client.c -o client

clear:
	rm master server client