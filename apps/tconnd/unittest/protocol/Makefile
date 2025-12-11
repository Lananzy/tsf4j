#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1.1.1 2009/03/03 11:25:18 hardway Exp $
#  @author $Author: hardway $
#  @date $Date: 2009/03/03 11:25:18 $
#  @version $Revision: 1.1.1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#

include ../../../../tsf4g.mk

PROTOCOL_LIB=libprotocol.a


PROTOCOL_CFILE=$(wildcard *.c)
PROTOCOL_OBJ=$(PROTOCOL_CFILE:.c=.o)



#DEPEND_LIBS=$(TSF4G_LIB)/libtcltapi.a $(TSF4G_LIB)/libtsec.a $(TSF4G_LIB)/libtapp.a $(TSF4G_LIB)/libtbus.a $(TSF4G_LIB)/libtagentapi.a $(TSF4G_LIB)/libtlog.a \
#			  $(TSF4G_LIB)/libtloghelp.a $(TSF4G_LIB)/libtmng.a  $(TSF4G_LIB)/libtdr_xml.a \
#	  $(TSF4G_LIB)/libpal.a

.PHONY: all clean

all: $(PROTOCOL_LIB) 

-include $(PROTOCOL_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<



$(PROTOCOL_LIB): $(PROTOCOL_OBJ)
	ar r $(PROTOCOL_LIB) $^

clean:
	$(RM) *.o  *.d
	$(RM) $(PROTOCOL_LIB)

