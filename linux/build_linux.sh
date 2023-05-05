#!/bin/bash

BUILDROOT_VER="2023.02"
LINUX_VER="5.15.43"

BUILDROOT_CONFIG="buildroot_conf.config"
LINUX_CONFIG="linux_conf.config"

if [ ! -d "buildroot" ]; then
    echo "Buildroot folder not found. Downloading..."
    buildroot_tar=buildroot-$BUILDROOT_VER.tar.gz
    wget https://buildroot.org/downloads/$buildroot_tar
    tar -xzvf $buildroot_tar
    rm $buildroot_tar
    mv buildroot-$BUILDROOT_VER buildroot
fi

cd buildroot
make qemu_riscv64_virt_defconfig
cp ../$BUILDROOT_CONFIG .config

make linux-menuconfig -j $(nproc)

cp ../$LINUX_CONFIG output/build/linux-$LINUX_VER/.config

make -j $(nproc)

cd ..

echo "Build completed successfully."
