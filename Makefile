CC = gcc


TARGET = dashboard 
DASH = dashboard.c

LINKEDLIBS = $(shell ldconfig -Np)

ifeq ($(findstring librt.so, $(LINKEDLIBS)), )
FLAGS = -g -lrt -lcurses -Wall -Wextra -std=gnu99 
else
FLAGS = -g -lcurses -Wall -Wextra -std=gnu99 
endif 

SRC = $(wildcard src/*.c)
SYS = $(wildcard system/*.c)
DIS = $(wildcard display/*.c)
UTIL = $(wildcard src/util/*.c)

$(TARGET): $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL)
	$(CC) $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL) -o $(TARGET) $(FLAGS)

install:
	cp $(TARGET) /usr/bin

uninstall:
	rm /usr/bin/$(TARGET)

clean:
	rm $(TARGET)
