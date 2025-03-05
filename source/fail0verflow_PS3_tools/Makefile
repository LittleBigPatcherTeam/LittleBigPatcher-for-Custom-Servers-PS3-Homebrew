TOOLS	=	unself
COMMON	=	tools.o fail0verflow_PS3_tools_aes.o fail0verflow_PS3_tools_sha1.o
DEPS	=	Makefile tools.h fail0verflow_PS3_tools_types.h

CC	=	gcc
CFLAGS	=	-g -O2 -Wall -W
LDFLAGS =	-lz

OBJS	= $(COMMON) $(addsuffix .o, $(TOOLS))

all: $(TOOLS)

$(TOOLS): %: %.o $(COMMON) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON) $(LDFLAGS)

$(OBJS): %.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(OBJS) $(TOOLS)
