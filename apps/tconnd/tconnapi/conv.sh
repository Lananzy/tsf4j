#!/bin/bash

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}
export PATH

tdr --xml2dr -o tframehead.tdr ../../../lib_src/tsec/tsecbasedef.xml tframehead.xml
tdr --xml2h ../../../lib_src/tsec/tsecbasedef.xml tframehead.xml
rm -f tsecbasedef.h
mv -f tframehead.h ../../../include/apps/tconnapi
tdr --xml2c -o tframehead.c ../../../lib_src/tsec/tsecbasedef.xml tframehead.xml
