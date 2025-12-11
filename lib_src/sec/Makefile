#  @file $RCSfile: Makefile,v $
#    Makefile for tsf4g-security module
#    $Id: Makefile,v 1.1.1.1 2008/05/28 07:35:00 kent Exp $
#  @author $Author: kent $
#  @date $Date: 2008/05/28 07:35:00 $
#  @version 1.0
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
	$(RM) *.o *.d core *.log*

