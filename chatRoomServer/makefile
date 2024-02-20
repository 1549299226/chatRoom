src=*.c
Server:${src}
	gcc ${src} -o Server -lmysqlclient -ljson-c

.PHONE:all clean

all:Server
	./Server

clean:
	rm -f Server