#!/bin/bash

BUILDROOT_VER="2023.02.2"
LINUX_VER="5.15.43"

BUILDROOT_CONFIG="rv64gcemu_defconfig"

if [ ! -d "buildroot" ]; then
    echo "Buildroot folder not found. Downloading..."
    buildroot_tar=buildroot-$BUILDROOT_VER.tar.gz
    wget https://buildroot.org/downloads/$buildroot_tar
    tar -xzvf $buildroot_tar
    rm $buildroot_tar
    mv buildroot-$BUILDROOT_VER buildroot
fi

mkdir -p overlay/
echo $PWD
cd buildroot
echo $PWD
make defconfig BR2_DEFCONFIG=../$BUILDROOT_CONFIG

make -j$(nproc)

