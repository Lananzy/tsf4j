include ../../tsf4g.mk

TAGENTAPI_CFILE=$(wildcard *.c)
TAGENTAPI_OBJ=$(TAGENTAPI_CFILE:.c=.o)
TAGENTAPI_LIB=$(TSF4G_LIB)/libtagentapi.a
TAGENTAPI_SO=$(TSF4G_LIB)/libtagentapi.so

CFLAGS += -DVERSION="\"1.2.1\""

LIBS  +=  $(TSF4G_LIB)/libtdr_xml.a $(TSF4G_LIB)/libpal.a $(TSF4G_LIB)/libcomm.a $(TSF4G_XML_LIBS)

ifeq ($(debug), 1)
	CFLAGS += -D__SD_DEBUG__ -D__LOG4C_DEBUG__
endif

.PHONY: all clean

all: $(TAGENTAPI_LIB) $(TAGENTAPI_SO)
	cp $(TAGENTAPI_OBJ)   $(TSF4G_PACKLIBDIR)

-include $(TAGENTAPI_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(DEBUG) $< -o $@


$(TAGENTAPI_LIB): $(TAGENTAPI_OBJ) 
	ar r $@ $^
	

$(TAGENTAPI_SO): $(TAGENTAPI_OBJ) 
	$(CC) -o $@ $(SHARED) $(CFLAGS) $(DEBUG) $^ $(LDPATH) $(LIBS)  

clean:
	$(RM) *.o *.d
	$(RM)  $(TAGENTAPI_LIB) $(TAGENTAPI_SO)

