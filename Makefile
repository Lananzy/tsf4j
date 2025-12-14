#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.8 2008/01/17 02:38:54 huwu Exp $
#  @author $Author: huwu $
#  @date $Date: 2008/01/17 02:38:54 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  tsf4g.mk

CLEANTARGET = clean
TESTTARGET = test 

MODS = pal comm tdr log sec tbus
MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))
MODSTEST =$(patsubst %, %.$(TESTTARGET), $(MODS))

DEPS = expat scew CUnit

all: $(MODS)

.PHONY:  all dep clean $(MODS) $(MODSCLEAN) $(MODSTEST)

$(MODS): $(DEPS)
	@echo "Begin to make tsf4g module: '$@' ......" 
	cd $(TSF4G_SRC)/$@ && $(MAKE);
	@echo "Finish to  make tsf4g module: '$@'" 

$(MODSCLEAN):
	@echo "Begin to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......" 
	cd $(TSF4G_SRC)/$(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 

$(CLEANTARGET): $(MODSCLEAN)

$(MODSTEST):
	@echo "Begin to do unit-test for tsf4g module: '$(patsubst %.$(TESTTARGET),%, $@)' ......" 
	cd $(TSF4G_SRC)/$(patsubst %.$(TESTTARGET),%, $@) && $(MAKE) $(TESTTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(TESTTARGET),%, $@)'" 

$(TESTTARGET): $(MODSTEST)

$(DEPS): 
	@echo "Begin to make tsf4g module: '$@' ......" 
	cd $(TSF4G_DEPS)/$@ &&  ./configure --prefix=$(TSF4G_HOME) --exec-prefix=$(TSF4G_HOME) && $(MAKE) && $(MAKE) install;
	@echo "Finish to  make tsf4g module: '$@'" 

install:
	@echo "tsf4g need not install anything now."
