SRC := aesdsocket.c aesdhelper.c
TARGET = aesdsocket
OBJS := $(SRC:.c=.o)
CC := $(CROSS_COMPILE)gcc
# CFLAGS += -g -O0
CFLAGS += -pthread
# LDFLAGS += -lpthread
all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS) 

clean:
	-rm -f *.o $(TARGET) *.elf *.map
