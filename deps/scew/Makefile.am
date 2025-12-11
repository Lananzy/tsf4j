#
# $Id: Makefile.am,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
#
# Author: Aleix Conchillo Flaque <aleix@member.fsf.org>
# Date:   Mon Nov 04, 2002 20:29
#
# Copyright (C) 2002, 2003, 2004 Aleix Conchillo Flaque
#

SUBDIRS = doc etc win32 scew examples

EXTRA_DIST = INSTALL.WIN32

.PHONY: doc

if DOC

doc:
	(cd doc && $(MAKE) doc)

else

doc:
	@echo "Doxygen documentation generation not enabled";

endif
