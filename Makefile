CC     := mpicc
CFLAG  := -std=c99 -Wall -Wextra -O3
INC    := -Iinclude -ISimpleDecomp/include
LIB    := -lm
SRCDIR := src SimpleDecomp/src
OBJDIR := obj
SRC    := $(shell find $(SRCDIR) -type f -name *.c)
OBJ    := $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))
DEP    := $(patsubst %.c,$(OBJDIR)/%.d,$(SRC))
TARGET := a.out

help:
	@echo "all   : create \"$(TARGET)\""
	@echo "clean : remove \"$(TARGET)\" and object files under \"$(OBJDIR)\""
	@echo "help  : show this message"

all: $(TARGET)

clean:
	$(RM) -r $(OBJDIR) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAG) $^ -o $@ $(LIB)

$(OBJDIR)/%.o: %.c
	@if [ ! -e $(dir $@) ]; then \
		mkdir -p $(dir $@); \
	fi
	$(CC) $(CFLAG) -MMD $(INC) -c $< -o $@

-include $(DEP)

.PHONY : all clean help

