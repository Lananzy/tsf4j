# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.3 2008/08/07 01:55:40 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008/08/07 01:55:40 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk
CFLAGS +=  -I$(TSF4G_INC)/obus -I/usr/local/include/scew

OBUS_CFILE=$(wildcard *.c)
OBUS_OBJ=$(OBUS_CFILE:.c=.o)
OBUS_LIB=$(TSF4G_LIB)/libobus.a
OBUS_SO=$(TSF4G_LIB)/libobus.so

.PHONY: all clean

all: $(OBUS_LIB) $(OBUS_SO)

-include $(OBUS_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $< 

$(OBUS_LIB):	$(OBUS_OBJ)
	ar r $(OBUS_LIB) $^


$(OBUS_SO): $(OBUS_OBJ)
	$(CC) -o $(OBUS_SO) $(SHARED) $(CFLAGS) $(DEBUG) $^ $(TSF4G_XML_LIBS)

clean:
	$(RM) *.o  *.d
	$(RM) $(OBUS_LIB) $(OBUS_SO)

