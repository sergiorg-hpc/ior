#!/bin/bash

IORDIR=`cd $(dirname "$0") && pwd`
BUILDDIR="$IORDIR/build"
BINDIR=$IORDIR/../build/bin
UMMAPIO=$IORDIR/../ummap-io/build

export MPICC=mpicc
export CC=gcc
export CFLAGS="-I/usr/include/mpi -I$UMMAPIO/include"
export LDFLAGS="-L$UMMAPIO/lib"
export LIBS="-lummapio"

cd $IORDIR

if [[ `ls $IORDIR/src/Makefile 2>/dev/null` == "" ]]
then
    ./bootstrap
    ./configure --with-mmap --prefix=$BUILDDIR
fi

make && cp $IORDIR/src/ior $BINDIR/ior.out

