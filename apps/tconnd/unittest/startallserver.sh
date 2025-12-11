#!/bin/bash

./tconnd --id=6.1.1.1 --conf-file=./tconnd.xml --tlogconf=./tconnd_log.xml --bus-key=2009 --daemon start

cd server;./tserver --id=6.1.1.2 --conndid=6.1.1.1 --tlogconf=./tserver_log.xml --use-bus --bus-key=2009 --echofactor=1 --daemon start;cd -

