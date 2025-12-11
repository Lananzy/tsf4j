#!/bin/sh

# Fisrt of all, you should set PATH to xml2c and xml2h properly

if [ $# -ne 1 ]
    then
	        echo "Usage: generate.sh  pathoftdrtools"
			        exit 1
					fi

					echo $1

					PATH=$1:${PATH}
					
#PATH=.:${HOME}/tsf4g/tools/:${PATH}
export PATH

tdr --xml2h  tbus_macros.xml tbus_comm.xml tbus_desc.xml

tdr --xml2c tbus_macros.xml tbus_comm.xml tbus_desc.xml -o tbus_desc.c

tdr --xml2h tbus_head.xml

tdr --xml2c tbus_head.xml -o tbus_head.c

mv tbus_macros.h ../../include/tbus


