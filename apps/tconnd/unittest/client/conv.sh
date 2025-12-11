#!/bin/sh

# Fisrt of all, you should set PATH to xml2c and xml2h properly

PATH=$1:${HOME}/tsf4g/tools/

export PATH


tdr --xml2h -o tclient_def.h tclient_def.xml
tdr --xml2c -o tclient_def.c tclient_def.xml
