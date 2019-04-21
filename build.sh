#!/bin/bash

IORDIR=`cd $(dirname "$0") && pwd`
BUILDDIR="$IORDIR/build"
BINDIR=$IORDIR/../build/bin
UMMAPIO=$IORDIR/../ummap-io/build
MAKEFILE_CHECK=`ls $IORDIR/src/Makefile 2>/dev/null`

if [[ $1 == "make" ]]
then
    export CC=$2
    export MPICC=$2
    export CFLAGS="-I$UMMAPIO/include"
    export LDFLAGS="-L$UMMAPIO/lib"
    export LIBS="-lummapio"

    cd $IORDIR

    if [[ $MAKEFILE_CHECK == "" ]]
    then
        ./bootstrap
        ./configure --with-mmap --without-gpfs --prefix=$BUILDDIR
    fi

    make CC="$CC" MPICC="$MPICC" && cp $IORDIR/src/ior $BINDIR/ior.out
else
    cd $IORDIR

    if [[ $MAKEFILE_CHECK != "" ]]
    then
        make clean
    fi
fi

