#
# $Id: Makefile.am,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
#
# Author: Aleix Conchillo Flaque <aleix@member.fsf.org>
# Date:   Sun Jul 06, 2003 04:51
#
# Copyright (C) 2003 Aleix Conchillo Flaque
#

DOXYGEN_LOG = doxygen.log
API_DIR = api

if DOC

doc:
	mkdir -p $(API_DIR);
	@-$(DOXYGEN);

else

doc:
	@echo "Doxygen documentation generation not enabled"

endif

clean:
	rm -rf $(API_DIR) $(DOXYGEN_LOG);
