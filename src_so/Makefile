CC = gcc
CFLAGS += -fPIC

OBJS = $(patsubst %.c, %.o, $(wildcard ./*.c)) 
TARGET = libSoMyTree.so

$(TARGET): $(OBJS)
	$(CC) -shared $(CFLAGS) $^ -o $@
	cp $(TARGET) /usr/lib/

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)