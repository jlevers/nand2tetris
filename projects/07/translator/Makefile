CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-format-security -g -fsanitize=undefined
LDLIBS = -lm -L../../lib/ -Wl,-rpath=../../lib/ -lhashtable -lmcheck
OBJFILES := parser.o code_writer.o util.o
OBJDIR := build
SRCDIR := src
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := translator

$(TARGET): $(OBJS) $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test clean

test: $(OBJS)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/test.o $(SRCDIR)/test.c
	$(CC) $(CFLAGS) $(OBJDIR)/test.o $(SRC) -o $@
	./$@

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*.gch $(TARGET) test $(SRCDIR)/test/**/*.asm
