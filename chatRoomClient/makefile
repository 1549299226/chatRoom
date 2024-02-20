src=*.c
Client:${src}
	gcc ${src} -o Client -lmysqlclient -ljson-c

.PHONE:all clean

all:Client
	./Client

clean:
	rm -f Client

