# @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2008/06/23 08:51:58 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/06/23 08:51:58 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.2, tab=4
#  @note Platform: Linux

include ../../tsf4g.mk

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)
TDR_LIB=$(TSF4G_LIB)/libtdr.a
TDR_SO=$(TSF4G_LIB)/libtdr.so
TDR_LIB_WITH_DB=$(TSF4G_LIB)/libtdr_dbms.a
TDR_SO_WITH_DB=$(TSF4G_LIB)/libtdr_dbms.so

TDR_LIB_WITH_XML=$(TSF4G_LIB)/libtdr_xml.a
TDR_SO_WITH_XML=$(TSF4G_LIB)/libtdr_xml.so

TDR_LIB_WITH_DBMS_XML=$(TSF4G_LIB)/libtdr_dbms_xml.a
TDR_SO_WITH_DBMS_XML=$(TSF4G_LIB)/libtdr_dbms_xml.so


TDR_DBMS_WRAPPER_PATH = ./dbms
TDR_DBMS_WRAPPER_CFILE = $(wildcard $(TDR_DBMS_WRAPPER_PATH)/*.c)
TDR_DBMS_WRAPPER_OBJ = $(TDR_DBMS_WRAPPER_CFILE:.c=.o)

TDR_XML_WRAPPER_PATH = ./xml_metalib
TDR_XML_WRAPPER_CFILE = $(wildcard $(TDR_XML_WRAPPER_PATH)/*.c)
TDR_XML_WRAPPER_OBJ = $(TDR_XML_WRAPPER_CFILE:.c=.o)


 

TDR_CFLAGS = $(CFLAGS) 

.PHONY: all clean

all: $(TDR_LIB) $(TDR_SO) $(TDR_LIB_WITH_DB) $(TDR_SO_WITH_DB) $(TDR_LIB_WITH_XML) $(TDR_SO_WITH_XML) $(TDR_LIB_WITH_DBMS_XML) \
 $(TDR_SO_WITH_DBMS_XML) 
	cp $(TDR_OBJ)  $(TSF4G_PACKLIBDIR)
	cp $(TDR_DBMS_WRAPPER_OBJ) $(TSF4G_PACKLIBDIR)
	cp  $(TDR_XML_WRAPPER_OBJ)  $(TSF4G_PACKLIBDIR)

	
-include $(TDR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(TDR_CFLAGS)  $< -o $@

%.o: %.c %.d
	$(CC) -c $(TDR_CFLAGS)  $(DEBUG) $<

$(TDR_LIB):	$(TDR_OBJ)
	ar rsv $(TDR_LIB) $^ 
	
$(TDR_SO): $(TDR_OBJ)
	$(CC) -o $(TDR_SO) $(SHARED) $(TDR_CFLAGS) $(DEBUG)   $^  
	
$(TDR_LIB_WITH_DB):	$(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ)
	ar rsv $(TDR_LIB_WITH_DB) $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ)
	
$(TDR_SO_WITH_DB): $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ)
	$(CC) -o $(TDR_SO_WITH_DB) $(SHARED) $(TDR_CFLAGS) $(DEBUG)   $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(LDPATH) -lgcc  $(TSF4G_DBMS_LIB)

$(TDR_LIB_WITH_XML):	$(TDR_OBJ) $(TDR_XML_WRAPPER_OBJ)
	ar rsv $(TDR_LIB_WITH_XML) $(TDR_OBJ) $(TDR_XML_WRAPPER_OBJ)
	
$(TDR_SO_WITH_XML): $(TDR_OBJ) $(TDR_XML_WRAPPER_OBJ)
	$(CC) -o $(TDR_SO_WITH_XML) $(SHARED) $(TDR_CFLAGS) $(DEBUG)   $(TDR_OBJ) $(TDR_XML_WRAPPER_OBJ) $(LDPATH)  $(TSF4G_XML_LIBS)

$(TDR_LIB_WITH_DBMS_XML):	$(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ)
	ar rsv $(TDR_LIB_WITH_DBMS_XML) $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ)
	
	
$(TDR_SO_WITH_DBMS_XML): $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ)
	$(CC) -o $(TDR_SO_WITH_DBMS_XML) $(SHARED) $(TDR_CFLAGS) $(DEBUG)   $(TDR_OBJ) $(TDR_DBMS_WRAPPER_OBJ) $(TDR_XML_WRAPPER_OBJ) $(LDPATH) $(TSF4G_XML_LIBS) -lgcc  $(TSF4G_DBMS_LIB)


$(TDR_DBMS_WRAPPER_OBJ):
	(cd $(TDR_DBMS_WRAPPER_PATH); make)

$(TDR_XML_WRAPPER_OBJ):
	(cd $(TDR_XML_WRAPPER_PATH); make)
	

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_LIB)
	$(RM) $(TDR_SO)
	$(RM) $(TDR_LIB_WITH_DB)
	$(RM) $(TDR_SO_WITH_DB)
	$(RM) $(TDR_LIB_WITH_DBMS_XML)
	$(RM) $(TDR_SO_WITH_DBMS_XML)
	$(RM) $(TDR_LIB_WITH_XML)
	$(RM) $(TDR_SO_WITH_XML)
	(cd $(TDR_DBMS_WRAPPER_PATH); make clean)
	(cd $(TDR_XML_WRAPPER_PATH); make clean)
	
