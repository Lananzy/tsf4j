#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h -O . ../../lib_src/tlog/tlogfilterdef.xml tdumpconf.xml
tdr --xml2c -o tdumpconf.c ../../lib_src/tlog/tlogfilterdef.xml tdumpconf.xml

rm -f tlogfilterdef.h

