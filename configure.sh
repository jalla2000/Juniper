#!/bin/bash

LIBSPOTIFY="libspotify-0.0.8-linux6-i686"

# install libspotify into source tree per default
if [ -z "$SPOTIFY" ]; then
    SPOTIFY=local
fi

echo "Cleaning up..."
make clean 2>/dev/null
rm *.pro 2>/dev/null
rm Makefile 2>/dev/null

# build dependencies need to be installed first since qmake
# relies on pkgconfig files beeing present
if [ "$1" == "all" ]
then
    sudo aptitude install libqt4-dev build-essential libsndfile-dev libasound-dev libogg-dev lame
fi

echo -n "Checking for libspotify..."
if [ -f /usr/lib/libspotify.so ]; then
    echo "Found global libspotify"
elif [ -f libspotify/lib/libspotify.so ]; then
    echo "Found local libspotify"
    INSTDIR=`pwd`/libspotify
    SPOTIFY=local
else
    if [ ! -f $LIBSPOTIFY.tar.gz ]; then
        wget http://developer.spotify.com/download/libspotify/$LIBSPOTIFY.tar.gz
    fi
    tar xfz $LIBSPOTIFY.tar.gz
    case $SPOTIFY in
        "local")
            INSTDIR=`pwd`/libspotify
            ;;
        "global")
            INSTDIR=/usr
            SUDO=sudo
            ;;
    esac
    echo "Not found. Installing in $INSTDIR"
    cd $LIBSPOTIFY && $SUDO make install prefix=$INSTDIR && cd ..
fi

# we need to set PKG_CONFIG_PATH to find spotify unless it's
# installed in /usr
# additionally, we install spotify runtime libraries on 'make
# install'; if you don't like that compile with -rpath set, or
# use the LD_LIBRARY_PATH environment variable
if [ $SPOTIFY = "local" ]; then
    export PKG_CONFIG_PATH=$INSTDIR/lib/pkgconfig
    LIBSPOTIFY_INSTALLS='INSTALLS+=libspotify libspotify.path=/usr/lib libspotify.files=libspotify/lib/*.so.*'
fi


echo "Generating makefile..."
qmake -project -norecursive -o juniper.pro \
    QT+=network CONFIG+=link_pkgconfig PKGCONFIG+="alsa libspotify sndfile" \
    INSTALLS+=target target.path=/usr/bin/ $LIBSPOTIFY_INSTALLS \
    OBJECTS_DIR=build MOC_DIR=build RCC_DIR=build \
    UI_DIR=build UI_HEADERS_DIR=build UI_SOURCES_DIR=build \
    src/
echo "Generating meta objects..."
qmake

echo "All done!"
