#!/bin/bash
echo "Cleaning up..."
make clean 2>/dev/null
rm *.pro 2>/dev/null
rm Makefile 2>/dev/null
echo "Generating makefile..."
qmake -project
echo "QT += network" >> juniper.pro
echo "Generating meta objects..."
qmake
echo "Adding libspotify compile flags..."
# warning. this works, but creates a file named "cat". TODO: correct
cat make.libflags > cat Makefile > /tmp/juniper.make.temp && mv /tmp/juniper.make.temp Makefile
sed "s/-pipe -O2 -Wall -W/\$(SPOTFLAGS) -pipe -O2 -Wall -W/" Makefile > tmp.tmp && mv tmp.tmp Makefile
echo "All done!"