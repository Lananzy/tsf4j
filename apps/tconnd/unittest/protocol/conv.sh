#!/bin/sh

# Fisrt of all, you should set PATH to xml2c and xml2h properly

PATH=$1:${HOME}/tsf4g/tools/

export PATH

tdr --xml2dr -d -o protocol.tdr protocol.xml
tdr --xml2h -o protocol.h protocol.xml
tdr --xml2c protocol.xml -o protocol.c
