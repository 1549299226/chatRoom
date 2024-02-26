src=*.c
Client:${src}
	gcc ${src} -o Client -lmysqlclient -ljson-c

client:${src}
	gcc -g ${src} -o client -lmysqlclient -ljson-c

.PHONE:all clean gdb

all:Client
	./Client

gdb:client
	gdb ./client

clean:
	rm -f Client
	rm -f client