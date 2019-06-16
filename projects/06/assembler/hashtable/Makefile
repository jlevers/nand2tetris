CC = gcc
CFLAGS = -Wall -Werror
OBJFILES := hash_table.o prime.o
OBJDIR := build
OBJS := $(addprefix $(OBJDIR)/,$(OBJFILES))
SRCDIR := src
SRC := $(addprefix $(SRCDIR)/,$(OBJFILES:.o=.c))
TARGET := libhashtable.so

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ -lm

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -fpic -c -o $@ $<

clean:
	rm $(OBJDIR)/*.o $(TARGET)
