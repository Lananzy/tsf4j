#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1 2009/01/23 01:53:28 sean Exp $
#  @author $Author: sean $
#  @date $Date: 2009/01/23 01:53:28 $
#  @version $Revision: 1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#
CLEANTARGET = clean
SUBMODS = master_tconnd mod_tconnd
SUBMODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(SUBMODS)) 

.PHONY: all $(CLEANTARGET) $(SUBMODS) 

all: $(SUBMODS)
$(CLEANTARGET): $(SUBMODSCLEAN)
	
$(SUBMODS):
	@echo "Begin to make sub module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make sub module: '$@'" 

$(SUBMODSCLEAN): 
	@echo "Begin to clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@)  && make $(CLEANTARGET);
	@echo "Finish to  clean sub module: '$(patsubst %.$(CLEANTARGET),%, $@)'" 
