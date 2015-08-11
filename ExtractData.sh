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
      ./wgrib -v $x | egrep "(:APCP:|:SPFH:|:TMP:|:UGRD:|:VGRD:|:DLWRF:|:DSWRF:|:PRES:sfc:)" | ./wgrib -i -nh $x -o "$x.dat"
  done
else
  echo "Please specify year!"
  exit 1
fi
