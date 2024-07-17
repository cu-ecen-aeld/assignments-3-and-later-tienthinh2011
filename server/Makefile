SRC := aesdsocket.c aesdhelper.c
TARGET = aesdsocket
OBJS := $(SRC:.c=.o)
CC := $(CROSS_COMPILE)gcc
CFLAGS += -g -O0 #for debugging 
CFLAGS += -pthread
LDFLAGS += -lrt
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS) 

clean:
	-rm -f *.o $(TARGET) *.elf *.map
