#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define NLDAS_SIZE 103936
#define LA1 25.0625
#define LO1 -124.9375
#define LA2 52.9375
#define LO2 -67.0625
#define DI 0.125
#define DJ 0.125
#define NI 464
#define NJ 224

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

    int leap;

    leap = IsLeapYear (year);

    return days[leap][month] + mday;
}

int main (int argc, char *argv[])
{
    typedef struct data_field
    {
	double value;
	FILE *input_file;
	char filename[60];
	char field_name[15];
    } data_field;

    data_field field[9];

    int ind_i, ind_j;
    int	ind;

    time_t time_start, time_end;
    time_t rawtime;
    struct tm *timeinfo;
    struct tm timeinfo_start, timeinfo_end;

    int i, j, k;
    double lon = -(78.14083333), lat = 40.6125;
    double e_s, w_s, w;

    double prcp;
    double tmp;
    double spfh;
    double wind;
    double dlwrf;
    double dswrf;
    double pres;

    float temp;

    int jday;
    FILE *output_file;

    timeinfo_start.tm_year = 2009;
    timeinfo_start.tm_mon = 1;
    timeinfo_start.tm_mday = 1;
    timeinfo_start.tm_hour = 0;
    timeinfo_start.tm_min = 0;
    timeinfo_start.tm_sec = 0;

    timeinfo_end.tm_year = 2009;
    timeinfo_end.tm_mon = 1;
    timeinfo_end.tm_mday = 2;
    timeinfo_end.tm_hour = 0;
    timeinfo_end.tm_min = 0;
    timeinfo_end.tm_sec = 0;

    strcpy (field[0].field_name, "tmp");
    strcpy (field[1].field_name, "apcp");
    strcpy (field[2].field_name, "ugrd");
    strcpy (field[3].field_name, "vgrd");
    strcpy (field[4].field_name, "dlwrf");
    strcpy (field[5].field_name, "dswrf");
    strcpy (field[6].field_name, "pres");
    strcpy (field[7].field_name, "spfh");

    output_file = fopen ("Data/met.dat", "w");

    time_start = timegm(&timeinfo_start);
    time_end = timegm (&timeinfo_end);

    ind_i = rint ((lon - LO1) / DI) + 1;
    ind_j = rint ((lat - LA1) / DJ) + 1;

    ind = 1 + (ind_i - 1) + (ind_j - 1) * NI;

    printf ("I = %d, J = %d, ind = %d\n", ind_i, ind_j, ind);

    for (rawtime = time_start; rawtime <= time_end; rawtime = rawtime + 60*60)
    {
	timeinfo = gmtime(&rawtime);
	jday = doy (timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday);

	printf ("%2.2d/%2.2d/%4.4d %2.2d:00Z\n", timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_year,timeinfo->tm_hour);

	for (i = 0; i < 8; i++)
	{
	    sprintf (field[i].filename, "Data/%4.4d/%3.3d/NLDAS_FORA0125_H.A%4.4d%2.2d%2.2d.%2.2d00.002.grb.%s", timeinfo->tm_year, jday, timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, field[i].field_name);
	    field[i].input_file = fopen (field[i].filename, "rb");
	    if (field[i].input_file == NULL)
	    {
		printf ("Error when opening %s.\n", field[i].filename);
		exit (1);
	    }

	    fseek (field[i].input_file, (long int) ((ind - 1) * 4), SEEK_SET);
	    fread (&temp, 4L, 1, field[i].input_file);
	    field[i].value = (double) temp;
	    fclose (field[i].input_file);
	}

	tmp = field[0].value;
	prcp = field[1].value / 3600.;
	wind = sqrt (field[2].value * field[2].value + field[3].value * field[3].value);
	dlwrf = field[4].value;
	dswrf = field[5].value;
	pres = field[6].value;
	spfh = field[7].value;

	e_s = 611.2 * exp (17.67 * (tmp - 273.15) / (tmp - 273.15 + 243.5));
	w_s = 0.622 * e_s / (pres - e_s);
	w = spfh / (1.0 - spfh);

	fprintf (output_file, "%4.4d-%2.2d-%2.2d %2.2d:00\t", timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour);
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
