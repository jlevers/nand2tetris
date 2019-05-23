# The compiler to use
CC = gcc

# Compiler flags
# -g		enables debugging with gdb
# -Wall turns on most compiler warnings
CFLAGS = -g -Wall

# Executable to build
TARGET = assembler

all: $(TARGET)

$(TARGET): parser.o
	$(CC) $(CFLAGS) -o assembler parser.o

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c
