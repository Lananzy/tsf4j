#!/bin/sh

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

tdr --xml2h tapp_basedata_def.xml 
tdr --xml2c -o tapp_basedata_def.c tapp_basedata_def.xml 
tdr --xml2h tapp_rundata_timer_def.xml
tdr --xml2c -o tapp_rundata_timer_def.c tapp_rundata_timer_def.xml


