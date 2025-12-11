#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h -O . ../../lib_src/tlog/tlogfilterdef.xml  ../../lib_src/tlog/tlognetdef.xml logdump_conf_desc.xml
tdr --xml2c -o logdump_conf_desc.c ../../lib_src/tlog/tlogfilterdef.xml ../../lib_src/tlog/tlognetdef.xml logdump_conf_desc.xml

rm -f tlogfilterdef.h tlognetdef.h
