#!/bin/sh

# Bash script to convert read grid files and write into binary files using
#   using wgrib 
# Author: Yuning Shi (yshi.at.psu.edu)

DATA_DIR="Data"

if [ "$1" != "" ]; then
  # Specify year from command line argument
  YEAR=$1

  files=$(ls $DATA_DIR/$YEAR/*/NLDAS*.grb | sort -d)
  for x in $files
    do
      echo $x
      ./wgrib -s $x | grep ":TMP:2 m" | ./wgrib -i -nh $x -o "$x.tmp"
      ./wgrib -s $x | grep ":PRES:sfc" | ./wgrib -i -nh $x -o "$x.pres"
      ./wgrib -s $x | grep ":SPFH:" | ./wgrib -i -nh $x -o "$x.spfh"
      ./wgrib -s $x | grep ":UGRD:" | ./wgrib -i -nh $x -o "$x.ugrd"
      ./wgrib -s $x | grep ":VGRD:" | ./wgrib -i -nh $x -o "$x.vgrd"
      ./wgrib -s $x | grep ":APCP:" | ./wgrib -i -nh $x -o "$x.apcp"
      ./wgrib -s $x | grep ":DLWRF:" | ./wgrib -i -nh $x -o "$x.dlwrf"
      ./wgrib -s $x | grep ":DSWRF:" | ./wgrib -i -nh $x -o "$x.dswrf"
  done
else
  echo "Please specify year!"
  exit 1
fi
