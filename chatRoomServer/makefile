src=*.c
Server:${src}
	gcc ${src} -o Server -lmysqlclient -ljson-c

server:${src}
	gcc -g ${src} -o server -lmysqlclient -ljson-c

.PHONE:all clean gdb

all:Server
	./Server

gdb:server
	gdb ./server

clean:
	rm -f Server
	rm -f server