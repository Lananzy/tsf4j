#!/bin/sh

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

if [ ! -d "../../include/tlog/" ]
then
	mkdir ../../include/tlog/
fi

tdr --xml2h tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml
mv tlogdatadef.h ../../include/tlog/tlogdatadef.h
mv tlogfilterdef.h ../../include/tlog/tlogfilterdef.h
mv tlogfiledef.h ../../include/tlog/tlogfiledef.h
mv tlognetdef.h ../../include/tlog/tlognetdef.h
mv tlogvecdef.h ../../include/tlog/tlogvecdef.h
mv tloganydef.h ../../include/tlog/tloganydef.h
mv tlogbindef.h ../../include/tlog/tlogbindef.h
mv tlog_priority_def.h ../../include/tlog/tlog_priority_def.h
mv tlog_category_def.h ../../include/tlog/tlog_category_def.h
mv tlogconf.h ../../include/tlog/tlogconf.h

tdr --xml2c -o tlogconf.c tlogdatadef.xml tlog_priority_def.xml tlogfilterdef.xml tlogfiledef.xml tlognetdef.xml tlogvecdef.xml tloganydef.xml tlogbindef.xml tlog_category_def.xml tlogconf.xml

#tdr -C tlogdef.xml -o tlogdef.c


