#!/bin/bash

BUILDROOT_VER="2023.02"
LINUX_VER="5.15.43"

BUILDROOT_CONFIG="config_rv64gc_emu_buildroot"
LINUX_CONFIG="config_rv64gc_emu_linux"

if [ ! -d "buildroot" ]; then
    echo "Buildroot folder not found. Downloading..."
    buildroot_tar=buildroot-$BUILDROOT_VER.tar.gz
    wget https://buildroot.org/downloads/$buildroot_tar
    tar -xzvf $buildroot_tar
    rm $buildroot_tar
    mv buildroot-$BUILDROOT_VER buildroot
fi

cp $BUILDROOT_CONFIG buildroot/.config
cd buildroot
make oldconfig

make -j $(nproc)

cp ../$LINUX_CONFIG output/build/linux-$LINUX_VER/.config
make linux-update-config

make -j $(nproc)

cd ..

echo "Build completed successfully."
