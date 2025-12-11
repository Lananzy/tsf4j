#  @file $RCSfile: Makefile,v $
#    Makefile wrapper for tsf4g-tbus module
#  $Id: Makefile,v 1.2 2009/03/11 08:54:07 jacky Exp $
#  @author $Author: jacky $
#  @date $Date: 2009/03/11 08:54:07 $
#  @version $Revision: 1.2 $
#  @note Editor: Vim 6.3, Gcc 4.0.2, tab=4
#  @note Platform: Linux


include ../../tsf4g.mk

TARGETS = \
	SHELL \

all: $(TARGETS)

SHELL:
	@./generate.sh $(TSF4G_TOOLS)
	$(RM) *.o  *.d
	@$(MAKE) -f Makefile.real
	$(RM) *.o  *.d
	@$(MAKE) -f Makefile_r.real

#------------------------------------------------------
clean:
	@$(MAKE) -f Makefile.real clean
	@$(MAKE) -f Makefile_r.real clean

