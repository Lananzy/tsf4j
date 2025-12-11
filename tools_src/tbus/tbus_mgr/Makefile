#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-tbus channel information manager tool module
#  $Id: Makefile,v 1.3 2008/11/20 09:52:42 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/11/20 09:52:42 $
#  @version $Revision: 1.3 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../../tsf4g.mk

TBUS_TOOL_CFILE=$(wildcard *.c)
TBUS_TOOL_OBJ=$(TBUS_TOOL_CFILE:.c=.o)
TBUS_MGR=tbusmgr
TBUS_CONF=tbusconf.sh
TBUS_MGR_XML=tbusmgr.xml


DEPLIBS += $(TSF4G_LIB)/libtbus.a $(TSF4G_LIB)/libtlog.a $(TSF4G_LIB)/libtdr_xml.a  $(TSF4G_LIB)/libtmng.a \
	$(TSF4G_LIB)/libtagentapi.a $(TSF4G_LIB)/libpal.a $(TSF4G_LIB)/libcomm.a -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic  -lpthread

#ifeq ($(release), 1)
#	STATIC = -static -O3
#	LIBS += $(TSF4G_LIB)/libscew.a  $(TSF4G_LIB)/libexpat.a
#endif

CFLAGS += -I$(TSF4G_LIBSRC)

.PHONY: all clean

all: $(TBUS_MGR)

-include $(TBUS_TOOL_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $<

$(TBUS_MGR): $(TBUS_TOOL_OBJ)
	$(CC) $(STATIC) -o $@ $^ $(LDPATH) $(DEPLIBS) 
	@/bin/cp $(TBUS_MGR) $(TSF4G_TOOLS)/$(TBUS_MGR)
#	@/bin/cp $(TBUS_CONF) $(TSF4G_TOOLS)/$(TBUS_CONF)
#	@/bin/cp $(TBUS_CONF) $(TSF4G_TOOLS)/$(TBUS_CONF)
#	@/bin/cp $(TBUS_MGR_XML) $(TSF4G_TOOLS)/$(TBUS_MGR_XML)

clean:
	$(RM) core *.o  *.d
#	$(RM) $(TBUS_MGR) $(TSF4G_TOOLS)/$(TBUS_MGR) $(TSF4G_TOOLS)/$(TBUS_CONF) $(TSF4G_TOOLS)/$(TBUS_MGR_XML)

