#  @file $RCSfile: Makefile,v $
#    Makefile wrapper for tsf4g-tbus module
#  $Id: Makefile,v 1.2 2008/09/09 03:07:39 steve Exp $
#  @author $Author: steve $
#  @date $Date: 2008/09/09 03:07:39 $
#  @version $Revision: 1.2 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


include ../../tsf4g.mk

TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@./generate.sh $(TSF4G_TOOLS)
	@$(MAKE) -f Makefile.real

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean

