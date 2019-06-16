CC = gcc
CFLAGS = -Wall -Werror
LDLIBS = -lm -L./hashtable/ -Wl,-rpath=./hashtable/ -lhashtable
OBJFILES := encoder.o parser.o main.o symboltable.o
OBJDIR := build
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRCDIR := src
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := assembler

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o $(SRCDIR)/*.gch $(TARGET)
