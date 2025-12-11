#  general description of this module
#  $Id: Makefile,v 1.4 2008/11/20 09:55:19 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/11/20 09:55:19 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)




TDR_TOOLS_CFLAGS = $(CFLAGS) $(TSF4G_DBMS_INCLUDE)

TDR_DR2XML=$(TSF4G_TOOLS)/tdr2xml
TDR_TDR=$(TSF4G_TOOLS)/tdr
TDR_META2TAB=$(TSF4G_TOOLS)/meta2tab

TDR_TOOLS_BIN=$(TDR_DR2XML) $(TDR_TDR)  $(TDR_META2TAB)
TDR_TDR2XML_OBJS = tdr_dr2xml.o tgetopt.o tdr_tools.o
TDR_TDR_OBJS = tdr.o tgetopt.o tdr_tools.o
TDR_META2TAB_OBJS = tdr_meta2tab.o tgetopt.o tdr_tools.o

TDR_IMPORT_LIBS = $(TSF4G_LIB)/libtdr_xml.a   -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic 
TDR_IMPORT_LIBS_DBMS = $(TSF4G_LIB)/libtdr_dbms_xml.a    -Wl,-Bstatic -lscew -lexpat -Wl,-Bdynamic   $(TSF4G_DBMS_LIB)


.PHONY: all clean

all: $(TDR_TOOLS_BIN)

-include $(TDR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(TDR_TOOLS_CFLAGS)  $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS)  $(TDR_TOOLS_CFLAGS) $<

	
$(TDR_DR2XML): $(TDR_TDR2XML_OBJS)
	$(CC) -o $@  $(TDR_TOOLS_CFLAGS)   $^  $(LDPATH) $(TDR_IMPORT_LIBS)
	
$(TDR_TDR): $(TDR_TDR_OBJS)
	$(CC) -o $@  $(TDR_TOOLS_CFLAGS)   $^ $(LDPATH)  $(TDR_IMPORT_LIBS)


$(TDR_META2TAB): $(TDR_META2TAB_OBJS)
	$(CC) -o $@  $(TDR_TOOLS_CFLAGS)    $^ $(LDPATH)  $(TDR_IMPORT_LIBS_DBMS) 
	
clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_TOOLS_BIN)
