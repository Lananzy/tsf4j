#!/bin/bash

#stop server first
cd server;./tserver --id=6.1.1.2  stop;cd -

./tconnd --id=6.1.1.1 stop
