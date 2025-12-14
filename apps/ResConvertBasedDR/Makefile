#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.2 2008/02/03 06:42:43 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/02/03 06:42:43 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux



CLEANTARGET = clean


MODS = libresloader resprint testlibresloader 
MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))


all: $(MODS)

.PHONY: all clean $(MODS) $(MODSCLEAN) 

$(MODS): 
	@echo "Begin to make  module: '$@' ......" 
	cd $@ && $(MAKE);
	@echo "Finish to  make  module: '$@'" 

$(MODSCLEAN):
	@echo "Begin to clean  module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......" 
	cd $(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean  module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 

$(CLEANTARGET): $(MODSCLEAN)


dep:
	@echo "No depends building needed, just using make to build it. "

install:
	@echo "resconv need not install anything now."
