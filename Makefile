
TARGET			:= rwmem
OBJS			:= rwmem.o

CC              := $(CROSS_COMPILE)gcc
CXX             := $(CROSS_COMPILE)g++
CFLAGS			:= -Wall -pipe

.PHONY:	clean

all:	$(TARGET)

clean:
	@rm -rf $(TARGET) $(OBJS)

$(TARGET):	$(OBJS)
	$(CC) -o $@ $^ $(LFALGS)

