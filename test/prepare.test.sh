#!/bin/bash

echo "starting bind"
CURRENT_FOLDER=$(pwd)
echo $CURRENT_FOLDER
cd $CURRENT_FOLDER/test/docker_bind
bash run.sh
