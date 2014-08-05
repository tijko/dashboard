CC = gcc

FLAGS = -g -lcurses -lpthread -Wall

TARGET = dashboard 
DASH = dashboard.c

SRC = $(wildcard src/*.c)
SYS = $(wildcard system/*.c)
DIS = $(wildcard display/*.c)
UTIL = $(wildcard src/util/*.c)

$(TARGET): $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL)
	$(CC) $(DASH) $(DIS) $(SRC) $(SYS) $(UTIL) -o $(TARGET) $(FLAGS)

