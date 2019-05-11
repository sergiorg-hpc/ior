#!/bin/bash

IORDIR=`cd $(dirname "$0") && pwd`
BUILDDIR="$IORDIR/build"
BINDIR=$IORDIR/../build/bin
UMMAPIO=$IORDIR/../ummap-io/build
MAKEFILE_CHECK=`ls $IORDIR/src/Makefile 2>/dev/null`
CRAY_CHECK=`cc -V 2>&1 | grep Cray`

if [[ $1 == "make" ]]
then
    export CC=$2
    export MPICC=$2
    export CFLAGS="-I$UMMAPIO/include"
    export LDFLAGS="-L$UMMAPIO/lib"
    export LIBS="-lummapio"

    if [[ $CRAY_CHECK == "" ]]
    then
        export LIBS="$LIBS -pthread -lrt"
    fi

    cd $IORDIR

    if [[ $MAKEFILE_CHECK == "" ]]
    then
        ./bootstrap
        ./configure --with-mmap --with-ummap --without-gpfs --prefix=$BUILDDIR
    fi

    make CC="$CC" MPICC="$MPICC" && cp $IORDIR/src/ior $BINDIR/ior.out
else
    cd $IORDIR

    rm $BINDIR/ior.out 2>/dev/null

    if [[ $MAKEFILE_CHECK != "" ]]
    then
        make clean
    fi
fi

