#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h  -O . ../../../lib_src/tsec/tsecbasedef.xml tqqdef.xml tpdudef.xml
rm -f tsecbasedef.h
mv -f tqqdef.h ../../../include/apps/tcltapi/
mv -f tpdudef.h ../../../include/apps/tcltapi/
tdr --xml2c  -o tqqapidef.c ../../../lib_src/tsec/tsecbasedef.xml tqqdef.xml tpdudef.xml tqqapidef.xml

