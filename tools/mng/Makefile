include ../../tsf4g.mk

TMNG=$(TSF4G_TOOLS)/tmng
TAGENT=$(TSF4G_TOOLS)/agent
BIN=$(TMNG) #$(TAGENT)

TMNG_SRCOBJS= tmng_tool.o 
TAGENT_SRCOBJS=tmng_agent.o

SRCOBJS = $(TMNG_SRCOBJS)   $(TAGENT_SRCOBJS)

COMMLIB= -lpal -lcomm 

MNG_CFLAGES= $(CFLAGS)  -I$(TSF4G_LIBSRC)/tdr
PALLIB=-lpal

DRLIB= -ltdr_xml  $(TSF4G_XML_LIBS) 

MNGLIB=-Wl,-Bstatic  -ltapp -ltmng -ltbus -ltmngsvc -ltlog $(DRLIB)  $(COMMLIB)  -Wl,-Bdynamic  -lpthread


all:$(BIN)
	
$(TMNG): $(TMNG_SRCOBJS) 
	$(CC) $(CFLAGS) -o $@ $< $(LDPATH)  $(MNGLIB) 
	


$(TAGENT): $(TAGENT_SRCOBJS)
	$(CC) $(CFLAGS) -o $@ $<   $(LDPATH)   $(MNGLIB)  
	
$(SRCOBJS):%.o:%.c
	$(CC) -c $(MNG_CFLAGES) $(SHARED)  *.c 

clean:
	rm -f $(SRCOBJS) $(BIN) 
	
