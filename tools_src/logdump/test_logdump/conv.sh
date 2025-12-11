#!/bin/bash

# Fisrt of all, you should set PATH to tdr tools properly
PATH=$1:${PATH}
export PATH

tdr --xml2h  test_logdump_conf_desc.xml
tdr --xml2c  -o test_logdump_conf_desc.c test_logdump_conf_desc.xml

tdr --xml2h logic_log.xml
tdr --xml2c  -o logic_log.c logic_log.xml
tdr --xml2dr -o logic_log.tdr logic_log.xml
