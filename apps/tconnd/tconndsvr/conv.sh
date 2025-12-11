#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2dr -o tconnddef.tdr ../../../lib_src/tsec/tsecbasedef.xml tconnddef.xml
tdr --xml2h  -O . ../../../lib_src/tsec/tsecbasedef.xml tconnddef.xml
rm -f tsecbasedef.h
tdr --xml2c  -o tconnddef.c ../../../lib_src/tsec/tsecbasedef.xml tconnddef.xml
