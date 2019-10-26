INCLUDE_FILES = helper.h
BUILD_FILES = server.c client.c
TARGET_FILES = server client

CC = gcc

all: $(BUILD_FILES) $(INCLUDE_FILES)
	$(CC) server.c -o server -lpthread -std=c99 
	$(CC) client.c -o client -lpthread -std=c99

clean: 
	rm -f $(TARGET_FILES)