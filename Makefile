
TARGET			:= rwmem
OBJS			:= rwmem.o

CC              := gcc
CXX             := g++
CFLAGS			:= -Wall -pipe

.PHONY:	clean

all:	$(TARGET)

clean:
	@rm -rf $(TARGET) $(OBJS)

$(TARGET):	$(OBJS)
	$(CC) -o $@ $^ $(LFALGS)

