CC = gcc
CFLAGS = -Wall -Werror -g
OBJFILES := hash_table.o prime.o test.o
OBJDIR := build
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRCDIR := src
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := libhashtable.so

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $<

.PHONY: test clean

test:
	rm -f $(SRCDIR)/*.gch
	$(CC) $(CFLAGS) $(OBJDIR)/test.o $(wildcard $(SRCDIR)/hash_table.*) -o $(OBJDIR)/$@
	./$(OBJDIR)/$@

clean:
	rm -f $(OBJDIR)/*.o $(SRCDIR)/*.gch $(TARGET)
