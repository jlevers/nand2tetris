# The compiler to use
CC = gcc

# Compiler flags
# -g		enables debugging with gdb
# -Wall turns on most compiler warnings
CFLAGS = -g3 -Wall --std=c99

# Executable to build
TARGET = assembler

OBJECTS = main.o parser.o encoder.o

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

clean:
	rm *.o

# symboltable.o: symboltable.c symboltable.h
# 	$(CC) $(CFLAGS) -c symboltable.c
