/* LunarSummary: a stupid program to call Lunar::summaryPHL on behalf
 * of the PHP dvaa.org page.
 *
 */

#include <iostream>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ei.h>
#include "support.h"
#include "lunar-master/riseset3.h"
#include <unistd.h>
#include <sys/select.h>
#include "lunar-master/watdefs.h"
#include "lunar-master/date.h"
#include "lunar-master/lunar.h"
#include "lunar-master/afuncs.h"

#undef LS_DEBUG

const static double pi =
     3.1415926535897932384626433832795028841971693993751058209749445923078;

void usage(const char* pProg) {
  fprintf( stderr, "usage: %s [-lon <lon> -lat <lat> -tz <tz offset>] [-h]\n\nwhere\n"
                   "       lon = decimal longitude, west positive (-180.0 ... 180.0)\n"
                   "       lat = decimal latitude, north positive (-90.0 ... 90.0)\n"
                   "       tz = offset from UTC (-12 ... 12)\n"
                   "       -h to output in HTML-friendly format (<BR> vs. \\n, etc.)\n",
                   pProg);
  exit(1);
}

void erlcmd_send(char *response, size_t len)
{
    uint16_t be_len = htons(len - sizeof(uint16_t));
    memcpy(response, &be_len, sizeof(be_len));

    size_t wrote = 0;
    do {
        ssize_t amount_written = write(STDOUT_FILENO, response + wrote, len - wrote);
        if (amount_written < 0) {
          if (errno == EINTR)
            continue;
          exit(0);
        }

        wrote += amount_written;
    } while (wrote < len);
}

float ReverseFloat( const float inFloat )
{
   float retVal;
   char *floatToConvert = ( char* ) & inFloat;
   char *returnFloat = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnFloat[0] = floatToConvert[3];
   returnFloat[1] = floatToConvert[2];
   returnFloat[2] = floatToConvert[1];
   returnFloat[3] = floatToConvert[0];

   return retVal;
}

static void my_get_rise_set_times( double *rise_set, const int planet_no,
                  double jd,
                  const double observer_lat, const double observer_lon,
                  const char *vsop_data)
{
   int i;

                                    /* Mark both the rise and set times     */
                                    /* as -1,  to indicate that they've     */
                                    /* not been found.  Of course,  it may  */
                                    /* turn out that one,  or both,  do     */
                                    /* not occur during the given 24 hours. */
   rise_set[0] = rise_set[1] = -1;
                                    /* Compute the altitude for each hour:  */
   double anHour =  1. / 24.;
   jd = jd - anHour - anHour;

   for( i = 0; i < 24; i++)
      {
      int idx;
      double jd_riseset;

      jd_riseset = look_for_rise_set( planet_no, jd, jd + 1. / 24.,
                  observer_lat, observer_lon, vsop_data, &idx);

      if( idx != -1)
         rise_set[idx] = jd_riseset;
      jd += 1. / 24.;
      }
}


int main(int argc, char** argv) {
  bool doHtml = false;
  double lon = -75.15, lat = 40.0;  // Phila, PA
  int tz = 0, opt;                      // Phila, PA
  double jdate = 2458386.0;
  int planet = 15;
  char priv_path[1024] = "priv";
  char codepath[1024];

  while ((opt = getopt(argc, argv, "l:n:d:t:p:c:")) != -1) {
    switch (opt) {
      case 'l': lat = atof(optarg); break;
      case 'n': lon = atof(optarg); break;
      case 'd': jdate = atof(optarg); break;
      case 't': tz = atof(optarg); break;
      case 'p': planet = atoi(optarg); break;
      case 'c': strcpy(priv_path, optarg); break;
      default:
          fprintf(stderr, "Usage: %s [-criapdh] [file...]\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }
  sprintf(codepath, "%s/vsop.bin", priv_path);

  char *vsop_data = load_file_into_memory( codepath, NULL);

  int lun, days;
  char buf[256];
  double pdrs[2];
  double ndrs[2];
  double rs[2];
  double fraction;
  double rsd;
  int isSetting;
  double psr, pss, nsr, nss = 0.0;

  lat = lat * pi / 180.;
  lon = lon * pi / 180.;
  double tz_adjust = tz / 24.;

  my_get_rise_set_times(rs, planet, jdate, lat, lon, vsop_data);

  if (jdate <= rs[0]) {
    my_get_rise_set_times(pdrs, planet, jdate - 1., lat, lon, vsop_data);
    psr = pdrs[0];
    pss = pdrs[1];
    nsr = rs[0];
    nss = rs[1];
  } else if ((jdate > rs[0]) && (jdate < rs[1])) {
    my_get_rise_set_times(pdrs, planet, jdate - 1., lat, lon, vsop_data);
    my_get_rise_set_times(ndrs, planet, jdate + 1., lat, lon, vsop_data);
    psr = rs[0];
    pss = pdrs[1];
    nsr = ndrs[0];
    nss = rs[1];
  } else {
    my_get_rise_set_times(ndrs, planet, jdate + 1., lat, lon, vsop_data);
    psr = rs[0];
    pss = rs[1];
    nsr = ndrs[0];
    nss = ndrs[1];
  }

  char priseTime[256] = { 0 };
  char psetTime[256] = { 0 };
  char nriseTime[256] = { 0 };
  char nsetTime[256] = { 0 };

  fraction = psr + tz_adjust;
  my_full_ctime( priseTime, fraction, 0, tz);
  fraction = pss + tz_adjust;
  my_full_ctime( psetTime, fraction, 0, tz);
  fraction = nsr + tz_adjust;
  my_full_ctime( nriseTime, fraction, 0, tz);
  fraction = nss + tz_adjust;
  my_full_ctime( nsetTime, fraction, 0, tz);

//  fprintf(stderr, "the return was %lf - %lf for %lf\n\n", rs[0], rs[1], jdate);

  char resp[2048];
  int resp_index = sizeof(uint16_t);

  ei_encode_version(resp, &resp_index);
  ei_encode_map_header(resp, &resp_index, 4);
  ei_encode_atom(resp, &resp_index, "prev_rise");
  ei_encode_binary(resp, &resp_index, priseTime, strlen(priseTime));
  ei_encode_atom(resp, &resp_index, "prev_set");
  ei_encode_binary(resp, &resp_index, psetTime, strlen(psetTime));

  ei_encode_atom(resp, &resp_index, "next_rise");
  ei_encode_binary(resp, &resp_index, nriseTime, strlen(priseTime));
  ei_encode_atom(resp, &resp_index, "next_set");
  ei_encode_binary(resp, &resp_index, nsetTime, strlen(psetTime));

  erlcmd_send(resp, resp_index);
  
  exit(EXIT_SUCCESS);
}
