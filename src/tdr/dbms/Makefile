# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008/05/28 07:35:00 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)
TDR_DBMS_WRAPPER = libtdrdbmswrapper.a
TSF4G_LIBSRC  = $(TSF4G_HOME)/lib_src
TSF4G_DBMS_INCLUDE = -I/usr/local/mysql-5.0.67-linux-i686-glibc23/include
TSF4G_DBMS_LIB = -L/usr/local/mysql-5.0.67-linux-i686-glibc23/lib -lmysqlclient

TDR_IMPORT_LIBS=  /usr/local/lib 

TDR_CFLAGS= $(CFLAGS)  $(TSF4G_DBMS_INCLUDE) -I$(TSF4G_LIBSRC) 

.PHONY: all clean

all: $(TDR_DBMS_WRAPPER)

-include $(TDR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(TDR_CFLAGS)  $< -o $@

%.o: %.c %.d
	$(CC) -c $(TDR_CFLAGS)   $<

$(TDR_DBMS_WRAPPER):	$(TDR_OBJ)
	ar rsv $(TDR_DBMS_WRAPPER) $^ 

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_DBMS_WRAPPER)


