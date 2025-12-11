#!/bin/bash

cd client;./tclient --online=1000 --package=128 --max-speed=10 --conf-file=./tclient_def_data.xml --timer=100 --daemon start;cd -
cd client1;./tclient1  --online=1000 --package=128 --max-speed=10 --conf-file=./tclient_def_data.xml --timer=100 --daemon start;cd -
