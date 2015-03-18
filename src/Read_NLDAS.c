#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <time.h>
#include <string.h>
#include <math.h>

/* NLDAS-2 grid information */
#define NLDAS_SIZE  103936
#define LA1	    25.0625
#define LO1	    -124.9375
#define LA2	    52.9375
#define LO2	    -67.0625
#define DI	    0.125
#define DJ	    0.125
#define NI	    464
#define NJ	    224

int IsLeapYear (int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int doy (int year, int month, int mday)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };

    int             leap;

    leap = IsLeapYear (year);

    return days[leap][month] + mday;
}

int main (int argc, char *argv[])
{
    typedef struct data_field
    {
        double          value;
        FILE           *input_file;
        char            filename[60];
        char            field_name[15];
    } data_field;

    data_field      field[9];

    int             ind_i, ind_j;
    int             ind;

    time_t          time_start, time_end;
    time_t          rawtime;
    struct tm       timeinfo_start = { 0, 0, 0, 0, 0, 0 };
    struct tm       timeinfo_end = { 0, 0, 0, 0, 0, 0 };
    struct tm      *timeinfo;

    int             i, j, k;
    double          lon = -999.0, lat = -999.0;

    double          prcp;
    double          tmp;
    double          spfh;
    double          wind;
    double          dlwrf;
    double          dswrf;
    double          pres;
    double          e_s, w_s, w;
    double	    tx, tn;
    double	    daily_solar;
    double	    daily_prcp;
    double	    rhx, rhn;
    double	    daily_wind;

    float           temp;

    int             data_year = -999;
    int             year, month, mday;
    int             jday;

    FILE           *output_file;

    int             c;

    enum model	    { PIHM, CYCLES };
    enum model	    mode;
    /*
     * Get command line options
     */
    while (1)
    {
        static struct option long_options[] = {
            {"start",	required_argument, 0, 'a'},
            {"end",	required_argument, 0, 'b'},
            {"lat",	required_argument, 0, 'c'},
            {"lon",	required_argument, 0, 'd'},
            {"year",	required_argument, 0, 'e'},
	    {"model",	required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int             option_index = 0;

        c = getopt_long (argc, argv, "a:b:c:d:e:f:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 'a':
                sscanf (optarg, "%d-%d-%d", &year, &month, &mday);
                timeinfo_start.tm_year = year - 1900;
                timeinfo_start.tm_mon = month - 1;
                timeinfo_start.tm_mday = mday;
                break;
            case 'b':
                sscanf (optarg, "%d-%d-%d", &year, &month, &mday);
                timeinfo_end.tm_year = year - 1900;
                timeinfo_end.tm_mon = month - 1;
                timeinfo_end.tm_mday = mday;
                break;
            case 'c':
                sscanf (optarg, "%lf", &lat);
                break;
            case 'd':
                sscanf (optarg, "%lf", &lon);
                if (lon > 180)
                    lon = lon - 360.0;
                break;
            case 'e':
                sscanf (optarg, "%d", &data_year);
                break;
	    case 'f':
		if (strcasecmp ("PIHM", optarg) == 0)
		    mode = PIHM;
		else if (strcasecmp ("CYCLES", optarg) == 0)
		    mode = CYCLES;
		else
		{
		    printf ("Model %s is not recognised\n", optarg);
		    abort ();
		}
		break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort ();
        }
    }

    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    /*
     * Check command line options
     */
    if ((timeinfo_start.tm_year == 0 || timeinfo_end.tm_year == 0) && data_year == -999)
    {
        printf ("ERROR: Please either specify desired start and end times using --start and --end, or specify desired year using --year.\n");
        exit (1);
    }
    else if ((timeinfo_start.tm_year == 0 || timeinfo_end.tm_year == 0) && data_year > -999)
    {
        timeinfo_start.tm_year = data_year - 1900;
        timeinfo_start.tm_mon = 0;
        timeinfo_start.tm_mday = 1;
        timeinfo_end.tm_year = data_year + 1 - 1900;
        timeinfo_end.tm_mon = 0;
        timeinfo_end.tm_mday = 1;
    }
    if (lat == -999 || lon == -999)
    {
        printf ("ERROR: Please specify desired location using --lat and --lon.\n");
        exit (1);
    }
    else if (lat < LA1 || lat > LA2)
    {
        printf ("ERROR: latitude out of range (25.0625N ~ 52.9375N)\n");
        exit (1);
    }
    else if (lon < LO1 || lon > LO2)
    {
        printf ("ERROR: longitude out of range (124.9375W ~ 67.0625W)\n");
        exit (1);
    }

    printf ("Read NLDAS data for %lf N, %lf W, from %2.2d/%2.2d/%4.4d to %2.2d/%2.2d/%4.4d\n\n", lat, lon, timeinfo_start.tm_mon + 1, timeinfo_start.tm_mday, timeinfo_start.tm_year + 1900, timeinfo_end.tm_mon + 1, timeinfo_end.tm_mday, timeinfo_end.tm_year + 1900);
    if (mode == PIHM)
	printf ("Generate forcing file for PIHM.\n");
    else if (mode == CYCLES)
	printf ("Generate weather file for Cycles.\n");

    sleep (2);

    /* Specify names of desired NLDAS fields */
    strcpy (field[0].field_name, "tmp");
    strcpy (field[1].field_name, "apcp");
    strcpy (field[2].field_name, "ugrd");
    strcpy (field[3].field_name, "vgrd");
    strcpy (field[4].field_name, "dlwrf");
    strcpy (field[5].field_name, "dswrf");
    strcpy (field[6].field_name, "pres");
    strcpy (field[7].field_name, "spfh");

    /* Open output file */
    output_file = fopen ("Data/met.dat", "w");
    fprintf (output_file, "%-16s\t", "TIME");
    fprintf (output_file, "%-11s\t", "PRCP");
    fprintf (output_file, "%-6s\t", "SFCTMP");
    fprintf (output_file, "%-5s\t", "RH");
    fprintf (output_file, "%-6s\t", "SFCSPD");
    fprintf (output_file, "%-6s\t", "SOLAR");
    fprintf (output_file, "%-6s\t", "LONGWV");
    fprintf (output_file, "%-9s\n", "PRES");

    fprintf (output_file, "%-16s\t", "TS");
    fprintf (output_file, "%-11s\t", "kg/m2/s");
    fprintf (output_file, "%-6s\t", "K");
    fprintf (output_file, "%-5s\t", "%");
    fprintf (output_file, "%-6s\t", "m/s");
    fprintf (output_file, "%-6s\t", "W/m2");
    fprintf (output_file, "%-6s\t", "W/m2");
    fprintf (output_file, "%-9s\n", "Pa");

    time_start = timegm (&timeinfo_start);
    time_end = timegm (&timeinfo_end);


    /* Find the nearest NLDAS grid */
    ind_i = rint ((lon - LO1) / DI) + 1;
    ind_j = rint ((lat - LA1) / DJ) + 1;

    ind = 1 + (ind_i - 1) + (ind_j - 1) * NI;

    /* Read NLDAS files */
    for (rawtime = time_start; rawtime <= time_end; rawtime = rawtime + 60 * 60)
    {
        timeinfo = gmtime (&rawtime);
        jday = timeinfo->tm_yday + 1;

        printf ("%2.2d/%2.2d/%4.4d %2.2d:00Z\n", timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900, timeinfo->tm_hour);

        for (i = 0; i < 8; i++)
        {
	    /* Open input file */
            sprintf (field[i].filename, "Data/%4.4d/%3.3d/NLDAS_FORA0125_H.A%4.4d%2.2d%2.2d.%2.2d00.002.grb.%s", timeinfo->tm_year + 1900, jday, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, field[i].field_name);
            field[i].input_file = fopen (field[i].filename, "rb");
            if (field[i].input_file == NULL)
            {
                printf ("Error when opening %s.\n", field[i].filename);
                exit (1);
            }

	    /* Skip to locate the nearest NLDAS grid */
            fseek (field[i].input_file, (long int)((ind - 1) * 4), SEEK_SET);
	    /* Read in forcing */
            fread (&temp, 4L, 1, field[i].input_file);

            field[i].value = (double)temp;

            fclose (field[i].input_file);
        }

        tmp = field[0].value;
	/* Convert from total precipitation to precipitation rate */
        prcp = field[1].value / 3600.;
        wind = sqrt (field[2].value * field[2].value + field[3].value * field[3].value);
        dlwrf = field[4].value;
        dswrf = field[5].value;
        pres = field[6].value;
        spfh = field[7].value;

	/* Calculate relative humidity from specific humidity */
        e_s = 611.2 * exp (17.67 * (tmp - 273.15) / (tmp - 273.15 + 243.5));
        w_s = 0.622 * e_s / (pres - e_s);
        w = spfh / (1.0 - spfh);

        fprintf (output_file, "%4.4d-%2.2d-%2.2d %2.2d:00\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour);
        fprintf (output_file, "%-11.8lf\t", prcp);
        fprintf (output_file, "%-6.2lf\t", tmp);
        fprintf (output_file, "%-5.2lf\t", w / w_s * 100.0);
        fprintf (output_file, "%-5.2lf\t", wind);
        fprintf (output_file, "%-6.2lf\t", dswrf);
        fprintf (output_file, "%-6.2lf\t", dlwrf);
        fprintf (output_file, "%-9.2lf\n", pres);
    }

    return 0;
}
