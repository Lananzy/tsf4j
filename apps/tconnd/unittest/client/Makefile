#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2009/03/06 02:14:19 hardway Exp $
#  @author $Author: hardway $
#  @date $Date: 2009/03/06 02:14:19 $
#  @version $Revision: 1.2 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#

include ../../../../tsf4g.mk

APP_NAME=tclient

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)
APP_EXE=$(APP_NAME)


DEPEND_LIBS=../protocol/libprotocol.a $(TSF4G_LIB)/libtcltapi.a $(TSF4G_LIB)/libtsec.a $(TSF4G_LIB)/libtapp.a $(TSF4G_LIB)/libtbus.a $(TSF4G_LIB)/libtagentapi.a $(TSF4G_LIB)/libtlog.a \
			  $(TSF4G_LIB)/libtloghelp.a $(TSF4G_LIB)/libtmng.a  $(TSF4G_LIB)/libtdr_xml.a \
	  $(TSF4G_LIB)/libpal.a  $(TSF4G_LIB)/libcomm.a 

.PHONY: all clean

all: $(APP_EXE)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $<

$(APP_EXE): $(APP_OBJ)
	$(CC) -o $(APP_EXE) $(CFLAGS)  $^ $(DEPEND_LIBS) -lscew -lexpat -lpthread
	-cp $(APP_EXE) ../client1/tclient1

clean:
	$(RM) *.o  *.d
	$(RM) $(APP_EXE)

