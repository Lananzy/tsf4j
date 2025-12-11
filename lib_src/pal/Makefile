# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2009/03/31 09:31:07 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009/03/31 09:31:07 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

PAL_CFILE=$(wildcard *.c)
PAL_OBJ=$(PAL_CFILE:.c=.o)
PAL_LIB=$(TSF4G_LIB)/libpal.a
PAL_SO=$(TSF4G_LIB)/libpal.so

.PHONY: all clean

all: $(PAL_LIB) $(PAL_SO)
	cp $(PAL_OBJ)  $(TSF4G_PACKLIBDIR)

-include $(PAL_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS)  $<

$(PAL_LIB):	$(PAL_OBJ)
	ar r $(PAL_LIB) $^
	

$(PAL_SO): $(PAL_OBJ)
	$(CC) -o $(PAL_SO) $(SHARED) $(CFLAGS)  $^ 

clean:
	$(RM) *.o  *.d
	$(RM) $(PAL_LIB)
	$(RM) $(PAL_SO)

