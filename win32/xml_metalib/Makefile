# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2008/06/11 10:15:35 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/06/11 10:15:35 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)
TDR_XML_WRAPPER = libtdrxmlwrapper.a



TDR_CFLAGS = $(CFLAGS) -I  $(TSF4G_LIBSRC)  
.PHONY: all clean

all: $(TDR_XML_WRAPPER)

-include $(TDR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(TDR_CFLAGS)  $< -o $@

%.o: %.c %.d
	$(CC) -c $(TDR_CFLAGS)  $<

$(TDR_XML_WRAPPER):	$(TDR_OBJ)
	ar rsv $(TDR_XML_WRAPPER) $^ 

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_XML_WRAPPER)


