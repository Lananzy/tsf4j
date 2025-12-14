# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2008/02/03 06:43:14 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/02/03 06:43:14 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../resconv.mk
TSF4G_HOME=../../..
RESCONV_LIB=../lib
RESCONV_COMM=../comm
LIBRESLOEADER_CFILE=$(wildcard *.c) $(RESCONV_COMM)/ResConv.c
LIBRESLOEADER_OBJ=$(LIBRESLOEADER_CFILE:.c=.o)
LIBRESLOEADER_LIB=$(RESCONV_LIB)/libresloader.a
LIBRESLOEADER_SO=$(RESCONV_LIB)/libtdr.so

LIBRESLOEADER_CFLAGS = $(CFLAGS) -I $(TSF4G_HOME)/include
LIBRESLOEADER_DEPENDLIB = -L $(TSF4G_HOME)/lib -ltdr_xml

.PHONY: all clean

all: $(LIBRESLOEADER_LIB) $(LIBRESLOEADER_SO) 

-include $(LIBRESLOEADER_CFILE:.c=.d)

#%.d: %.c
#	$(CC) -MM $(LIBRESLOEADER_CFLAGS)  $< -o $@

$(RESCONV_COMM)/ResConv.o: $(RESCONV_COMM)/ResConv.c
	cd ../comm; make;
	
%.o: %.c 
	$(CC) -o $@ -c $(LIBRESLOEADER_CFLAGS)   $<

$(LIBRESLOEADER_LIB):	$(LIBRESLOEADER_OBJ)
	ar rsv $(LIBRESLOEADER_LIB) $^ 
	
$(LIBRESLOEADER_SO): $(LIBRESLOEADER_OBJ)
	$(CC) -o $(LIBRESLOEADER_SO) $(SHARED) $(LIBRESLOEADER_CFLAGS)    $^  $(LIBRESLOEADER_DEPENDLIB)
	

	
clean:
	$(RM) *.o  *.d
	$(RM) $(LIBRESLOEADER_LIB)
	$(RM) $(LIBRESLOEADER_SO)
	
