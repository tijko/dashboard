CC = gcc

FLAGS = -g -lcurses -lpthread -Wall

TARGET = dashboard 
DASH = dashboard.c

SRC = $(wildcard src/*.c)
SYS = $(wildcard system/*.c)
DIS = $(wildcard display/*.c)

$(TARGET): $(DASH) $(DIS) $(SRC) $(SYS)
	$(CC) $(DASH) $(DIS) $(SRC) $(SYS) -o $(TARGET) $(FLAGS)

