CC = gcc

FLAGS = -g -lcurses -Wall -Wextra -std=gnu99 

TARGET = dashboard 
DASH = dashboard.c

LINKEDLIBS = $(shell ldconfig -Np)

ifeq ($(findstring librt.so, $(LINKEDLIBS)), )
RT = -lrt
else
RT = 
endif 

SRC = $(wildcard src/*.c)
SYS = $(wildcard system/*.c)
DIS = $(wildcard display/*.c)
UTIL = $(wildcard src/util/*.c)

$(TARGET): $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL)
	$(CC) $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL) -o $(TARGET) $(FLAGS) $(RT)

install:
	cp $(TARGET) /usr/bin

uninstall:
	rm /usr/bin/$(TARGET)

clean:
	rm $(TARGET)
