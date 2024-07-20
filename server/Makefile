SRC := aesdsocket.c aesdhelper.c
TARGET = aesdsocket
OBJS := $(SRC:.c=.o)
#CC ?= gcc
#CFLAGS ?= -g -O0 #for debugging
CFLAGS ?= -g -Wall -Werror #for debugging
LDFLAGS ?= -lrt -lpthread
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.map
