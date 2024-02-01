CC = gcc
LDFLAGS = -L. -lSoMyTree

SRCS = main.c
OBJS = $(patsubst %.c, %.o, $(SRCS))

TARGET = myBinaryTree

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(TARGET)