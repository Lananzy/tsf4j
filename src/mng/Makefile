# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2008/12/09 03:32:27 hardway Exp $
#  @author $Author: hardway $
#  @date $Date: 2008/12/09 03:32:27 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

MNG_CFILE=$(wildcard *.c)
MNG_OBJ=$(MNG_CFILE:.c=.o)


COMMLIB = $(TSF4G_LIB)/libcomm.a
PALLIB= $(TSF4G_LIB)/libpal.a
DRLIB= $(TSF4G_LIB)/libtdr_xml.a -lexpat -lscew
TBUSLIB=-ltbus
LIBS =  $(DRLIB)  $(PALLIB) $(COMMLIB)

MNG_LIB=$(TSF4G_LIB)/libtmng.a
MNG_SO=$(TSF4G_LIB)/libtmng.so

.PHONY: all clean

all: $(MNG_LIB) $(MNG_SO)
	cp $(MNG_OBJ)  $(TSF4G_PACKLIBDIR)
	
.c.o:
	$(CC) -c $(CFLAGS) $(SHARED) $(DEBUG) *.c

$(MNG_LIB): $(MNG_OBJ)
	ar r $(MNG_LIB) $(MNG_OBJ)
	

$(MNG_SO): $(MNG_OBJ)
	$(CC) -o $(MNG_SO) $(SHARED) $(CFLAGS) $(DEBUG) $^ $(LDPATH) $(LIBS)


clean:
	$(RM) *.o 
	$(RM) $(MNG_LIB) $(MNG_SO)
