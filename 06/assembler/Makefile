CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -fsanitize=undefined
LDLIBS = -lm -L../../lib/ -Wl,-rpath=../../lib/ -lhashtable -lmcheck
OBJFILES := encoder.o parser.o symboltable.o
OBJDIR := build
SRCDIR := src
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := assembler

$(TARGET): $(OBJS) $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test clean

test: $(OBJS)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/test.o $(SRCDIR)/test.c
	$(CC) $(CFLAGS) $(OBJDIR)/test.o $(SRC) -o $@ $(LDLIBS)
	./$@

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*.gch $(TARGET) test
