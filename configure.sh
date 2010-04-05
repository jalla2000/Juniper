#!/bin/bash
echo "Cleaning up..."
make clean 2>/dev/null
rm *.pro 2>/dev/null
rm Makefile 2>/dev/null
echo "Generating makefile..."
qmake -project -norecursive -o juniper.pro
echo "QT += network" >> juniper.pro
echo "Generating meta objects..."
qmake
echo "Adding libspotify compile flags..."
cat make.libflags > /tmp/juniper.make.temp; cat Makefile >> /tmp/juniper.make.temp && mv /tmp/juniper.make.temp Makefile
sed -i "s/-pipe -O2 -Wall -W/\$(SPOTFLAGS) -pipe -O2 -Wall -W/" Makefile
echo -n "Checking for libspotify..."
if [ ! -e /usr/lib/libspotify.so ]
then
    echo "Not found. Installing..."
    if [ ! -e libspotify-0.0.3-linux6-i686 ]
    then
	wget http://developer.spotify.com/download/libspotify/libspotify-0.0.3-linux6-i686.tar.gz
	tar xfz libspotify-0.0.3-linux6-i686.tar.gz
	cd libspotify-0.0.3-linux6-i686 && sudo make install prefix=/usr/
    fi
else
    echo "Found!"
fi
if [ "$1" == "all" ]
then
    sudo aptitude install libqt4-dev g++ libsndfile-dev
fi
echo "All done!"
