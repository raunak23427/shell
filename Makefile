CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99
TARGET=simple-shell

$(TARGET): simple_shell.c
	$(CC) $(CFLAGS) -o $(TARGET) simple_shell.c

.PHONY: clean run

clean:
	rm -f $(TARGET)
    
run: $(TARGET)
	./$(TARGET)
