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

tdr --xml2h tsec_conf_desc.xml -o tsec_conf_desc.h
tdr --xml2h tsec_ipacl_desc.xml -o tsec_ipacl_desc.h

tdr --xml2c tsec_conf_desc.xml -o tsec_conf_desc.c
tdr --xml2c tsec_ipacl_desc.xml -o tsec_ipacl_desc.c

mv tsec_conf_desc.h ../../include/sec

