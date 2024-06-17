#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e #Exits the script if any command fails (returns a non-zero exit status).
set -u #Exits the script if an undefined variable is used.

# thinh added to got current dir of this file
CURRENTDIR=`pwd`

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    # deep clean (using mrproper)
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    # make defconfig => to generate .config
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    # make menuconfig (do not really need)
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} menuconfig
    # make all => build vmlinux: Build a kernel image for booting with QEMU #=> create vmlinux
    # make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
fi

echo "Adding the Image in outdir"
cp -r ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi
mkdir -p ${OUTDIR}/rootfs

# TODO: Create necessary base directories
cd "$OUTDIR/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
cd $OUTDIR/rootfs
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=`${CROSS_COMPILE}gcc -print-sysroot`
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 $OUTDIR/rootfs/lib/
cp -a $SYSROOT/lib64/ld-2.33.so* $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libm.so.6 $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libm-2.33.so $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libresolv.so.2 $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libresolv-2.33.so $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libc.so.6 $OUTDIR/rootfs/lib64/
cp -a $SYSROOT/lib64/libc-2.33.so $OUTDIR/rootfs/lib64/
# TODO: Make device nodes
cd $OUTDIR/rootfs
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1
# TODO: Clean and build the writer utility
cd $CURRENTDIR
make clean
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
cp writer $OUTDIR/rootfs/home/
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
mkdir -p $OUTDIR/rootfs/home/conf
cp finder.sh finder-test.sh $OUTDIR/rootfs/home/
cp conf/username.txt conf/assignment.txt $OUTDIR/rootfs/home/conf/
cp autorun-qemu.sh $OUTDIR/rootfs/home/
# TODO: Chown the root directory
cd ${OUTDIR}/rootfs
sudo chown -R root:root *
# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
cd ..
gzip initramfs.cpio
# mkimage -A arm -O linux -T ramdisk -d initramfs.cpio.gz uRamdisk
