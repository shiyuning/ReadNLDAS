CC = gcc
CFLAGS = -g -O0

SRCDIR = ./src
LIBS = -lm
SRCS_ = Read_NLDAS.c

EXECUTABLE = Read_NLDAS

SRCS = $(patsubst %,$(SRCDIR)/%,$(SRCS_))
OBJS = $(SRCS:.c=.o)

.PHONY: all clean help w3lib

all: wgrib Read_NLDAS

Read_NLDAS: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBS)

wgrib:
	$(CC) -o wgrib $(SRCDIR)/wgrib.c $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	@rm -f $(OBJS) $(EXECUTABLE)
