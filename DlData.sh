#!/bin/sh

# Bash script to download NLDAS-2 forcing data from NASA ftp
# Author: Yuning Shi (yshi.at.psu.edu)

DATA_DIR="Data"

if [ "$1" != "" ]; then
  # Specify year from command line argument
  YEAR=$1

  # Specify download target
  echo "Downloading Data ..."
  TARGET=$(printf "%s/%s/" "ftp://hydro1.sci.gsfc.nasa.gov/data/s4pa/NLDAS/NLDAS_FORA0125_H.002" $YEAR)

  # Download data
  wget -r -nH --cut-dirs=4 -P $DATA_DIR -A grb $TARGET
else
  echo "Please specify year!"
  exit 1
fi

exit 0
