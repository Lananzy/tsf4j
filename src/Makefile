#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.9 2009/02/09 08:13:36 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2009/02/09 08:13:36 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  ../tsf4g.mk

CLEANTARGET = clean

TSF4G_PACKLIB=$(TSF4G_LIB)/libtsf4g.a
TSF4G_PACKSO=$(TSF4G_LIB)/libtsf4g.so
TSF4G_LIB_TMP_DIR = tmp

MODS_DEPENDON_TDRTOOL=  tlog tsec tbus mng  tloghelp  
TSF4G_LIB_IMPORTLIB= $(LDPATH) $(TSF4G_DBMS_LIB) -lscew -lexpat

#ifeq ($(withsec), yes)
#	MODS_DEPENDON_TDRTOOL +=  tsec 
#	TSF4G_LIB_IMPORTLIB+=-lcrypto
#endif

TSF4G_LIB_IMPORTLIB+= $(LIBS)


MODS =pal comm  tdr  taa  $(MODS_DEPENDON_TDRTOOL)

#ifeq ($(withobus), yes)
	MODS += obus
#endif
MODS+=tapp


MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))


all: $(MODS) tsf4glib

.PHONY:  all  clean $(MODS) $(MODSCLEAN) 

$(MODS_DEPENDON_TDRTOOL):tdrtool

tdrtool:
	@if test ! -f $(TSF4G_TOOLS)/tdr  ; then \
		echo "Begin to make tdr tools at first "; \
		cd $(TSF4G_TOOLSRC)/tdr && $(MAKE) ; \
		echo "Finish to make tdr tools  "; \
	fi;
	
$(MODS): tsf4glib_pre
	@echo $(withobus)
	@echo "Begin to make tsf4g module: '$@' ......" 
	cd $@ && $(MAKE);  
	@echo "Finish to  make tsf4g module: '$@'" 

tsf4glib_pre:
	@if test ! -d $(TSF4G_PACKLIBDIR)   ; then 	mkdir $(TSF4G_PACKLIBDIR) ; else  $(RM) $(TSF4G_PACKLIBDIR)/*;	 fi;
	
tsf4glib:$(MODS)
	@echo "Begin to pack tsf4g library ......" 
	ar rsv $(TSF4G_PACKLIB) $(TSF4G_PACKLIBDIR)/*.o
	$(CC) -o $(TSF4G_PACKSO) $(SHARED) $(CFLAGS)  $(TSF4G_PACKLIBDIR)/*.o $(TSF4G_LIB_IMPORTLIB)
	@$(RM) -rf $(TSF4G_PACKLIBDIR)/*.o;
	@echo "Finish to pack tsf4g library ." 
	


$(MODSCLEAN):
	@echo "Begin to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......" 
	cd $(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 

tsf4glibclean:
	@$(RM) $(TSF4G_PACKLIBDIR)/*
	@if test  -f $(TSF4G_PACKLIB)   ; then 	$(RM) $(TSF4G_PACKLIB)  ;	fi;
	@if test  -f $(TSF4G_PACKSO)   ; then 	$(RM) $(TSF4G_PACKSO)  ;	fi;

	
$(CLEANTARGET): $(MODSCLEAN)  tsf4glibclean
