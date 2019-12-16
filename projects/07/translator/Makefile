CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wno-format-security -g -fsanitize=undefined
LDLIBS = -lm -L../../lib/ -Wl,-rpath=../../lib/ -lhashtable -lmcheck
OBJFILES := asm_constants.o code_writer.o parser.o util.o
OBJDIR := build
SRCDIR := src
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := translator

$(TARGET): $(OBJS) $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test clean

test: $(OBJS)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/test.o $(SRCDIR)/test.c
	$(CC) $(CFLAGS) $(OBJDIR)/test.o $(SRC) -o $(OBJDIR)/$@ $(LDLIBS)
	./$(OBJDIR)/$@

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*.gch $(TARGET) $(SRCDIR)/test/**/*.asm
