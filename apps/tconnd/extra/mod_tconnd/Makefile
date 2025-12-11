#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.1 2009/01/23 01:52:23 sean Exp $
#  @author $Author: sean $
#  @date $Date: 2009/01/23 01:52:23 $
#  @version $Revision: 1.1 $
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux
#


include ../../../../tsf4g.mk

TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@./conv.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean
