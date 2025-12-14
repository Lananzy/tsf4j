#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-bus relay manager module
#  $Id: Makefile,v 1.5 2008/11/27 06:58:32 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/11/27 06:58:32 $
#  @version $Revision: 1.5 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

RELAY_MGR_CFILE=$(wildcard *.c)
RELAY_MGR_OBJ=$(RELAY_MGR_CFILE:.c=.o)
RELAY_MGR=relaymgr
RELAY_XML=relaymgr.xml

CINC += -I../ -I../../ -I$(TSF4G_LIBSRC)
DEPLIBS += $(TSF4G_LIB)/libtbus.a $(TSF4G_LIB)/libtlog.a $(TSF4G_LIB)/libtdr_xml.a   \
 $(TSF4G_LIB)/libtagentapi.a	$(TSF4G_LIB)/libpal.a $(TSF4G_LIB)/libcomm.a -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic  -lpthread

#ifeq ($(release), 1)
#	STATIC = -static -O3
#	LIBS += $(TSF4G_LIB)/libexpat.a $(TSF4G_LIB)/libscew.a
#endif

.PHONY: all clean

all: $(RELAY_MGR)

-include $(RELAY_MGR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(RELAY_MGR): $(RELAY_MGR_OBJ) 
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 
	@/bin/cp $(RELAY_MGR) $(TSF4G_TOOLS)/$(RELAY_MGR)
#	@/bin/cp $(RELAY_XML) $(TSF4G_TOOLS)/$(RELAY_XML)

clean:
	$(RM) core *.o  *.d
#	$(RM) $(RELAY_MGR) $(TSF4G_TOOLS)/$(RELAY_MGR) ${TSF4G_TOOLS)/${RELAY_XML}

