CC = gcc

CFLAGS  = -Wall -Wextra

CPPFLAGS = -Iinclude/ -MMD

LDFLAGS = 

SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)

OBJDIR = build
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

DEP = $(OBJ:.o=.d)

TARGET = a.out

all: $(TARGET)

debug: CFLAGS += -g3 -ggdb -fsanitize=address
debug: CPPFLAGS += 
debug: all

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

-include $(DEP)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

.PHONY: clean
clean:
	rm -r $(OBJDIR)
