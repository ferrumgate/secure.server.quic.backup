#!/bin/bash

echo "starting bind"
CURRENT_FOLDER=$(pwd)
echo $CURRENT_FOLDER
cd $CURRENT_FOLDER/test/docker_bind
bash run.sh

cd $CURRENT_FOLDER/test/bin
./echo_server -c test.com,test.com.crt,test.com.key -L debug

#client start
#./echo_client -s 127.0.0.1:12345
