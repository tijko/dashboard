CC = gcc

TARGET = dashboard
DASH = dashboard.c

FLAGS = -g -lcurses -Wall -Wextra -std=gnu99 -lrt 

LIBCVER := $(shell ldd --version | grep "(GNU libc)" | cut -d '.' -f 2)
RT := $(shell if [[ $(LIBCVER) -lt 18 ]]; then echo -lrt; fi)

SRC := $(wildcard src/*/*.c)

$(TARGET): $(DASH) $(SRC)
	$(CC) $(DASH) $(SRC) -o $(TARGET) $(FLAGS) $(RT)

install:
	cp $(TARGET) /usr/bin

uninstall:
	rm /usr/bin/$(TARGET)

clean:
	rm $(TARGET)
