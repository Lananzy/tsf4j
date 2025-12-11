#!/bin/sh

if [ $# -ne 1 ] 
	then
		echo "Usage: generate.sh  pathoftdrtools"
		exit 1
fi

echo $1

PATH=$1:${PATH}

if [ ! -d "../../include/tsec/" ]
then
	mkdir ../../include/tsec/
fi

tdr --xml2h tsecbasedef.xml
mv tsecbasedef.h ../../include/tsec/tsecbasedef.h


