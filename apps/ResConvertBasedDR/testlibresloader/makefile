#  general description of this module
#  $Id: makefile,v 1.2 2008/02/03 06:44:05 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/02/03 06:44:05 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

TSF4G_HOME=../../..
RESCONV_COMM=../comm
RESCONV_LIB=../lib
RESPRINT_HOME=../resprint
TEST_CFILE=$(wildcard *.c) $(RESCONV_COMM)/ResConv.c $(RESPRINT_HOME)/resprint.c
TEST_OBJ=$(TEST_CFILE:.c=.o)
TEST_BIN= testlibresloader


TEST_CFLAGS = $(CFLAGS) -I $(TSF4G_HOME)/include -I $(RESPRINT_HOME)
TEST_DEPENDLIB = -L $(TSF4G_HOME)/lib -ltdr_xml -L $(RESCONV_LIB) -lresloader

.PHONY: all clean

all: $(TEST_BIN)

-include $(TEST_CFILE:.c=.d)



%.o: %.c 
	$(CC) -o $@ -c $(TEST_CFLAGS)   $<

$(TEST_BIN):	$(TEST_OBJ)
	$(CC) -o $@  $(TEST_CFLAGS)    $^  $(TEST_DEPENDLIB)
	

	
clean:
	$(RM) *.o 
	$(RM) $(TEST_BIN)

