#  general description of this module
#  $Id: makefile,v 1.2 2008/02/03 06:43:43 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/02/03 06:43:43 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

TSF4G_HOME=../../..
RESCONV_COMM=../comm
RESCONV_BIN=../bin
RESPRINT_CFILE=$(wildcard *.c) $(RESCONV_COMM)/ResConv.c
RESPRINT_OBJ=$(RESPRINT_CFILE:.c=.o)
RESPRINT_BIN= $(RESCONV_BIN)/resprint


RESPRINT_CFLAGS = $(CFLAGS) -I $(TSF4G_HOME)/include
RESPRINT_DEPENDLIB = -L $(TSF4G_HOME)/lib -ltdr_xml

.PHONY: all clean

all: $(RESPRINT_BIN)

-include $(RESPRINT_CFILE:.c=.d)



%.o: %.c 
	$(CC) -o $@ -c $(RESPRINT_CFLAGS)   $<

$(RESPRINT_BIN):	$(RESPRINT_OBJ)
	$(CC) -o $@  $(RESPRINT_CFLAGS)    $^  $(RESPRINT_DEPENDLIB)
	

	
clean:
	$(RM) *.o 
	$(RM) $(RESPRINT_BIN)

