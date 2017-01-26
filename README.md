Read_NLDAS
==========

Tools to download and extract NLDAS data.

Contact: Yuning Shi [(Send Email)](mailto:yshi@psu.edu)


INSTRUCTION:
------------

1. Download the source files and the Makefile to your work directory. In your work directory, type `make all` to compile all executables. Two executables will be generated: `read_nldas` and `wgrib`. In addition, there are two bash scripts, `dl_nldas.sh` and `extract_nldas.sh`.

2. Build NLDAS-2 data inventory:
    1. Download NLDAS-2 forcing data:
        The bash script `dl_nldas.sh` will download a whole year's NLDAS-2 forcing data into the Data directory.
        The syntax is:
        
        ```
        $ ./dl_nldas.sh YYYY
        ```
        
        `YYYY` indicates the year of data that you want to download.
    
    2. Extract the grib files:
        The downloaded NLDAS-2 forcing files are in grib format. The bash script `extract_nldas.sh` will extract the grib files and write the NLDAS-2 forcing fields into seperate binary files to build your data inventory.
        
        The syntax is:
        ```
        $ ./extract_nldas YYYY
        ```
        
        `YYYY` indicates the year of data that you want to extract.

3. Read NLDAS-2 forcing inventory and build forcing file: 

   This step will read the data inventory you built using `wgrib` and produce meteorology forcing data.
   The program will look into the NLDAS-2 forcing data and retrieve the time series from the nearest NLDAS grid to your desired lat/lon location.
   ```shell
   $ ./read_nldas --start YYYY-MM-DD --end YYYY-MM-DD [--year YYYY] --lat LAT --lon LON [--model MODEL]
   ```
   If a model parameter is not provided, the default format is PIHM forcing format.

   **EXAMPLE:**

   If you want to read the forcing from Jan 1 2009 to Feb 1 2009 at 40.6125N, 78.1408W and produce forcing for Cycles, you run
   ```
   $ ./read_nldas --start 2009-01-01 --end 2009-02-01 --lat 40.6125 --lon -78.1408 --model Cycles
   ```
   Note that W longitude is negative.

   Alternatively, if you want to read a whole year's forcing data, you can run
   ```
   $ ./read_nldas --year 2009 --lat 40.6125 --lon -78.1408 --model Cycles
   ```

ACKNOWLEDGMENT:
---------------
[Wgrib](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html) is an operational NCEP program to manipulate, inventory and decode GRIB files.
The source file `wgrib.c` is downloaded from [ftp://ftp.cpc.ncep.noaa.gov/wd51we/wgrib/wgrib.c](ftp://ftp.cpc.ncep.noaa.gov/wd51we/wgrib/wgrib.c).

