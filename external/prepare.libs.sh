#!/usr/bin/env bash
PROD="FALSE"
while getopts p flag; do
    case "${flag}" in
    p) PROD="TRUE" ;;
    esac
done

set -e
TMPFOLDER=/tmp/uv
CURRENTFOLDER=$(pwd)
rm -rf $TMPFOLDER
mkdir -p $TMPFOLDER

DESTFOLDER=$(pwd)/libs

######## install libuv ###############
cp libuv-v1.46.0.tar.gz $TMPFOLDER
#
echo $DESTFOLDER
cd $TMPFOLDER
tar zxvf libuv-v1.46.0.tar.gz
cd libuv-v1.46.0
sh autogen.sh
./configure --prefix=$DESTFOLDER

make
#make check
make install

####### install cmocka ############
cd $CURRENTFOLDER
cp cmocka-1.1.5.tar.xz $TMPFOLDER
cd $TMPFOLDER
tar xvf cmocka-1.1.5.tar.xz
cd cmocka-1.1.5
rm -rf CMakeCache.txt
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$DESTFOLDER -DCMAKE_BUILD_TYPE=Debug ../
make
make install

######### install hiredis ############
cd $CURRENTFOLDER
DESTFOLDER=$(pwd)/libs
cp hiredis-1.2.0.zip $TMPFOLDER
cd $TMPFOLDER
unzip hiredis-1.2.0.zip
cd hiredis-1.2.0
export PREFIX=$DESTFOLDER
make
make install

####### boringssl
cd $CURRENTFOLDER
DESTFOLDER=$(pwd)/libs
export PREFIX=$DESTFOLDER
if [ ! -d boringssl ]; then
    git clone https://boringssl.googlesource.com/boringssl
fi

cd $(pwd)/boringssl
git checkout e9f816b12b3e68de575d21e2a9b7d76e4e5c58ac

cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=1 -DCMAKE_INSTALL_PREFIX=$DESTFOLDER .

make
make install

################# lsquic
cd $CURRENTFOLDER
DESTFOLDER=$(pwd)/libs
export PREFIX=$DESTFOLDER
if [ ! -d lsquic ]; then
    git clone https://github.com/litespeedtech/lsquic.git
fi

cd lsquic
git checkout 836b0deadbfe9c7fa886a1cdfcc7e1db6675f934
git submodule init
git submodule update
export BORINGSSL=$CURRENTFOLDER/boringssl
cmake -DBORINGSSL_DIR=$BORINGSSL -DLSQUIC_SHARED_LIB=1 -DCMAKE_INSTALL_PREFIX=$DESTFOLDER .
make
make install

############ make ready ##############
#cd $CURRENTFOLDER
##chown -R hframed:hframed libs
