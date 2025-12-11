#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly

../../../../tools/tdr --xml2dr -o tconnddef.tdr ../../../../lib_src/tsec/tsecbasedef.xml ../../tconndsvr/tconnddef.xml
../../../../tools/tdr --xml2h  -O . ../../../../lib_src/tsec/tsecbasedef.xml ../../tconndsvr/tconnddef.xml
rm -f tsecbasedef.h
../../../../tools/tdr --xml2c  -o tconnddef.c ../../../../lib_src/tsec/tsecbasedef.xml ../../tconndsvr/tconnddef.xml
