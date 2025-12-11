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

APP_NAME=tserver

APP_CFILE=$(wildcard *.c)
APP_OBJ=$(APP_CFILE:.c=.o)
APP_EXE=$(APP_NAME)

LIBS=../protocol/libprotocol.a  $(TSF4G_LIB)/libtconnapi.a $(TSF4G_LIB)/libtsf4g.a -lscew -lexpat -lpthread

.PHONY: all clean

all: $(APP_EXE)

-include $(APP_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(APP_EXE): $(APP_OBJ)
	$(CC) -o $(APP_EXE) $(CFLAGS)  $^ $(LIBS)
	

clean:
	$(RM) *.o  *.d
	$(RM) $(APP_EXE)

