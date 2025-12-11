..\..\tools\tdr --xml2h tbus_macros.xml tbus_desc.xml

..\..\tools\tdr --xml2c tbus_macros.xml tbus_desc.xml -o tbus_desc.c

..\..\tools\tdr --xml2h tbus_head.xml

..\..\tools\tdr --xml2c tbus_head.xml -o tbus_head.c

pause
