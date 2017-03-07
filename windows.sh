#!/bin/bash

set -ev

PREVDIRECTORY=`pwd`
BASEDIR=$(dirname "$0")
cd $BASEDIR

# Figure out where the sourcetree is
SRC=`pwd | sed -e 's|/cygdrive/||'`"/src"

# We compile only for 64 bits for now
BITS=64 

# Prepare the compilation flags - We will compile while using MinGW-x64
CC=x86_64-w64-mingw32-gcc
ADD_CFLAGS="-O3 -m64 -fPIC -DPIC -D_XPG6 -D_FORTIFY_SOURCE=2 -I/usr/x86_64-w64-mingw32/include -Wl,-rpath=/usr/x86_64-w64-mingw32/lib"
if [ ! -z $MONETDBLITE_DEBUG ] ; then
	echo "Using debug flags"
	ADD_CFLAGS="-O0 -g -m64"
fi

# Remove previous installation data
rm -rf monetdb-java-lite/build
rm -f monetdb-java-lite/src/main/resources/libs/windows/*

# Recreate the directories
mkdir -p monetdb-java-lite/src/main/resources/libs/windows
mkdir -p src/embedded/incjni/

# Copy the Windows JNI headers
cp src/embedded/incwindows/* src/embedded/incjni/

# Patch sedscript for build/install/library paths
sed -e "s|%CC%|$CC|" -e "s|%ADD_CFLAGS%|$ADD_CFLAGS|" -e "s|%BITS%|$BITS|" -e "s|%PREFIX%|$SRC/../build|" -e "s|%SRCDIR%|$SRC|" src/embedded/windows/sedscript.tpl > src/embedded/windows/sedscript

# Re-update the monetdb_config.h file
rm -f src/monetdb_config.h
find src/ -name "Makefile" -delete
# This is copied from NT/monetdb_config.h.in in the packaging script
cp src/embedded/windows/monetdb_config.h.in src/
# PMC stands for "poor man's configure", it does something similar using the sedscript
sh src/embedded/windows/pmc.sh

# Now the tricky part, copy the fixed MinGW-x64 headers! (it may require root permissions!)
if [ ! -z $TRAVIS ] ; then
	cp src/embedded/windows/mingwheaders/intrin.h /usr/x86_64-w64-ming32/include
	cp src/embedded/windows/mingwheaders/stdlib.h /usr/x86_64-w64-ming32/include
	cp src/embedded/windows/mingwheaders/time.h /usr/x86_64-w64-ming32/include
	cp src/embedded/windows/mingwheaders/intrin-impl.h /usr/x86_64-w64-ming32/include/psdk_inc
fi

# Compile the shared library
cd src
rm -f config.status
touch Makefile.in config.status configure aclocal.m4 monetdb_config.h stamp-h1 monetdb_config.h.in

make -j

if [ $? -ne 0 ]
then
	echo "build failure"
fi

OFILES=`find common gdk mal/mal mal/modules mal/optimizer sql embedded mapisplit -name "*.lo" | tr "\n" " "`

$CC $ADD_CFLAGS -shared -fPIC -Wl,--export-all-symbols -o libmonetdb5.dll $OFILES -lws2_32 -lpthread -lpsapi

if [ ! -s libmonetdb5.dll ]
then
	echo "library file was not created, something went wrong"
fi

# Move the shared library to the resources directory, as well as the other dependent libraries
cd ..
cp src/embedded/windows/msvcr100.win$BITS/msvcr100-$BITS.dll monetdb-java-lite/src/main/resources/libs/windows/msvcr100.dll
mv src/libmonetdb5.dll monetdb-java-lite/src/main/resources/libs/windows/libmonetdb5.dll

# Build the jar with gradle
if [ -z $TRAVIS ] ; then
	cd monetdb-java-lite
	gradle build
fi

cd $PREVDIRECTORY
