#include <iostream>
#include <stddef.h>
#include <ei.h>
#include <unistd.h>
#include <sys/select.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "lunar-master/watdefs.h"
#include "lunar-master/date.h"
#include "lunar-master/lunar.h"
#include "lunar-master/afuncs.h"
#include "support.h"

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define VSOP_CHUNK 2767U

#define NEW_MOON 0
#define FIRST_QUARTER 1
#define FULL_MOON 2
#define LAST_QUARTER 3

const double days_per_julian_century = 36525.;

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



static double approx_solar_dist( double t)
{
   double temp, r[3], rval;

   t /= 10.;      /* cvt to julian millennia */
   temp = 6283.07585 * t;
   r[0] = 100013989. +
            1670700. * cos( 3.0984635 + temp) +
              13956. * cos( 3.05525 + temp * 2) +
               3084. * cos( 5.1985 + 77713.7715 * t);
   r[1] =    103019. * cos( 1.107490 + temp) +
               1721. * cos( 1.0644 + temp * 2.);
   r[2] =      4359. * cos( 5.7846 + temp);
   rval = r[0] + t * (r[1] + t * (r[2] + t));
   rval *= 1.e-8;
   rval *= AU_IN_KM;    /* put return value into KILOMETERS */
   return( rval);
}

int main(int argc, char** argv) {
  const double j2000 = 2451545.;    /* 1.5 Jan 2000 = JD 2451545 */
  const long double lunation = 29.530588853;
  int i, opt;
  double t,  t0, utc_time, original_t;
  double m, mp, f, e, max_date = 4000. * 365.25 + j2000;
  double lunar, dist, fund[N_FUND], rate = 29.5306, t_final;
  char buff[80];
  double k;
  double tz = 0.;
  FILE *log_file = NULL, *vsop_file, *data_file = NULL;
  char *vsop_tbuff, FAR *vsop_data;
  double previous_full, previous_first, previous_new, previous_last;
  double next_full, next_first, next_new, next_last;
  bool  before_phases = true;
      double last_phase_time;
      int last_phase_type, current_last_phase;
      char  phase[2048];
  char priv_path[1024] = "priv";
  char codepath[1024];

  previous_full = previous_first = previous_new = previous_last = 0L;
  next_full = next_first = next_new = next_last = 0L;

  while ((opt = getopt(argc, argv, "l:n:d:t:c:")) != -1) {
    switch (opt) {
      case 'd': original_t = (atof(optarg)); break;
      case 'c': strcpy(priv_path, optarg); break;
      case 't': tz = atof(optarg); break;
      default:
          fprintf(stderr, "Usage: %s [-criapdh] [file...]\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }
  sprintf(codepath, "%s/vsop.bin", priv_path);

  vsop_file = fopen(codepath, "rb");
  if( !vsop_file) {
    fprintf(stderr, "Couldn't open vsop.bin");
    return( -1);
  }
  vsop_tbuff = (char *)malloc( VSOP_CHUNK);
  vsop_data = (char *)malloc( VSOP_CHUNK * 22U);
  for( i = 0; i < 22; i++) {
    if( !fread( vsop_tbuff, VSOP_CHUNK, 1, vsop_file)) {
      printf( "Couldn't read VSOP data\n");
      free( vsop_tbuff);
      free( vsop_data);
      return( -2);
    }
    FMEMCPY( vsop_data + (unsigned)i * VSOP_CHUNK, vsop_tbuff, VSOP_CHUNK);
  }
  fclose( vsop_file);
  free( vsop_tbuff);

  t0 = t = original_t - lunation;
  k = floor(((t0-j2000) / 365.25) * 12.3685);
  double tz_adjust = tz / 24.;

  max_date = t0 + (lunation * 2.);

   t_final = max_date - 1.;

  while( t_final < max_date) {
    for( i = 0; i < 4; i++) {
      double t2, dlon_1, dlon_2, phase_angle, solar_lon, time_lag;
      static const char *phase_name[4] = {
              "New moon ",
              "1st qtr. ",
              "Full moon",
              "last qtr." };
      double t_centuries, t_cen2, t_cen3, t_cen4;

      t_centuries = k / 1236.85;       /* first approx */
      t = 2451550.09765 + 29.530588853 * k
           + (1.337e-4 - 1.5e-7 * t_centuries) * t_centuries * t_centuries;
      t_centuries = (t - j2000) / 36525.;
      t_cen2 = t_centuries * t_centuries;
      t_cen3 = t_cen2 * t_centuries;
      t_cen4 = t_cen3 * t_centuries;
      m = 2.5534 + 29.10535669 * k
                -  2.18e-5 * t_cen2
                -  1.1e-7 * t_cen3;
      mp = 201.5643 + 385.81693528 * k
                   +    .0107438 * t_cen2
                   +   1.239e-5 * t_cen3
                   -   5.8e-8 * t_cen4;
      f = 160.7108 + 390.67050274 * k
                  -   1.6541e-3 * t_cen2
                  -   2.27e-6 * t_cen3
                  +   1.1e-8 * t_cen4;
      m *= PI / 180.;
      f *= PI / 180.;
      mp *= PI / 180.;
      e = 1. - .002516 * t_centuries - 7.4e-6 * t_cen2;

      switch( i) {
        case NEW_MOON:
           t +=   -.40720 * sin( mp)
                  +.17241 * sin( m) * e
                  +.01608 * sin( mp + mp)
                  +.01039 * sin( f + f)
                  +.00739 * sin( mp - m) * e
                  -.00514 * sin(  mp + m) * e
                  +.00208 * sin( m + m) * e * e
                  -.00111 * sin( mp - f - f);
           break;
        case FIRST_QUARTER:
        case LAST_QUARTER:
           t +=   -.62801 * sin( mp)
                  +.17172 * sin( m) *  e
                  -.01183 * sin( mp + m) * e
                  +.00862 * sin( mp + mp)
                  +.00804 * sin( f + f)
                  +.00454 * sin( mp - m) * e
                  +.00204 * sin( m + m) * e * e
                  -.00180 * sin( mp - f - f);
           t += ((i == FIRST_QUARTER) ? .00306 : -.00306);
           break;
        case FULL_MOON:
           t +=   -.40614 * sin( mp)
                  +.17302 * sin( m) * e
                  +.01614 * sin( mp + mp)
                  +.01043 * sin( f + f)
                  +.00734 * sin( mp - m) * e
                  -.00515 * sin(  mp + m) * e
                  +.00209 * sin( m + m) * e * e
                  -.00111 * sin( mp - f - f);
           break;
        default:
           break;
      }
      phase_angle = (double)i * 90.;
      t_centuries = (t - j2000) / days_per_julian_century;
      time_lag = approx_solar_dist( t_centuries) / SPEED_OF_LIGHT;
      time_lag /= seconds_per_day * days_per_julian_century;
      lunar_fundamentals( vsop_data, t_centuries, fund);
      lunar_lon_and_dist( vsop_data, fund, &lunar, &dist, 0L);
      solar_lon = calc_vsop_loc( vsop_data, 3, 0, t_centuries - time_lag, 0.);
      solar_lon = solar_lon * 180. / PI - 180.;
      dlon_1 = lunar - solar_lon - phase_angle;
      while( dlon_1 < -180.) dlon_1 += 360.;
      while( dlon_1 >  180.) dlon_1 -= 360.;

      t2 = t - rate * dlon_1 / 360.;
      t_centuries = (t2 - j2000) / 36525.;
      lunar_fundamentals( vsop_data, t_centuries, fund);
      lunar_lon_and_dist( vsop_data, fund, &lunar, &dist, 0L);
      solar_lon = calc_vsop_loc( vsop_data, 3, 0, t_centuries - time_lag, 0.);
      solar_lon = solar_lon * 180. / PI - 180.;
      dlon_2 = lunar - solar_lon - phase_angle;
      while( dlon_2 < -180.) dlon_2 += 360.;
      while( dlon_2 >  180.) dlon_2 -= 360.;


      t_final = (t * dlon_2 - t2 * dlon_1) / (dlon_2 - dlon_1);
      utc_time = t_final - td_minus_utc( t_final) / seconds_per_day;
      my_full_ctime( buff, utc_time, 0, tz);

      if (before_phases && (t_final > original_t)) {
        before_phases = false;
        current_last_phase = i;
        bool front_half = ((t_final - last_phase_time) / 2.) > (t_final - original_t);
        sprintf(phase, "%s", "unset phase");
      }
      last_phase_type = i;


      if (before_phases) {
        switch( i) {
          case NEW_MOON:
             previous_new = utc_time + tz_adjust;
             break;
          case FIRST_QUARTER:
             previous_first = utc_time + tz_adjust;
             break;
          case LAST_QUARTER:
             previous_last = utc_time + tz_adjust;
             break;
          case FULL_MOON:
             previous_full = utc_time + tz_adjust;
             break;
          default:
             break;
        }
      } else {
        switch( i) {
          case NEW_MOON:
            if (!next_new)
              next_new = utc_time + tz_adjust;
            break;
          case FIRST_QUARTER:
            if (!next_first)
              next_first = utc_time + tz_adjust;
            break;
          case LAST_QUARTER:
            if (!next_last)
              next_last = utc_time + tz_adjust;
            break;
          case FULL_MOON:
            if (!next_full)
              next_full = utc_time + tz_adjust;
            break;
          default:
             break;
        }
      }
      double flrd_original_t = floor(original_t + 0.5 + tz_adjust);

//      fprintf(stderr, "\n\n%f : %f : %f\n\n", flrd_original_t, previous_full, next_full);

      if (floor(next_new + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "New");
      else if (floor(previous_new + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "New");
      else if (floor(next_full + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "Full");
      else if (floor(previous_full + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "Full");
      else if (floor(next_first + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "First Quarter");
      else if (floor(previous_first + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "First Quarter");
      else if (floor(next_last + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "Last Quarter");
      else if (floor(previous_last + 0.5) == flrd_original_t)
        sprintf(phase, "%s", "Last Quarter");
      else {
        switch( current_last_phase) {
          case NEW_MOON:
            sprintf(phase, "%s", "Waning Crescent");
            break;
          case FIRST_QUARTER:
            sprintf(phase, "%s", "Waxing Crescent");
            break;
          case LAST_QUARTER:
            sprintf(phase, "%s", "Waning Gibbous");
            break;
          case FULL_MOON:
            sprintf(phase, "%s", "Waxing Gibbous");
            break;
          default:
            break;
        }
      }

//      fprintf(stderr, "%s: %s  ", phase_name[i], buff);
//      if( i == 3)
//         fprintf(stderr,  "\n");
              
      k += .25;
    }
  }
  char resp[2048];
  char rebuff[2048];
  int resp_index = sizeof(uint16_t);

  ei_encode_version(resp, &resp_index);
  ei_encode_map_header(resp, &resp_index, 9);
  ei_encode_atom(resp, &resp_index, "phase");
  ei_encode_binary(resp, &resp_index, phase, strlen(phase));

  my_full_ctime( rebuff, previous_full, 0, tz);
  ei_encode_atom(resp, &resp_index, "previous_full");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, previous_new, 0, tz);
  ei_encode_atom(resp, &resp_index, "previous_new");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, previous_first, 0, tz);
  ei_encode_atom(resp, &resp_index, "previous_first");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, previous_last, 0, tz);
  ei_encode_atom(resp, &resp_index, "previous_last");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, next_new, 0, tz);
  ei_encode_atom(resp, &resp_index, "next_new");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, next_first, 0, tz);
  ei_encode_atom(resp, &resp_index, "next_first");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, next_last, 0, tz);
  ei_encode_atom(resp, &resp_index, "next_last");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  my_full_ctime( rebuff, next_full, 0, tz);
  ei_encode_atom(resp, &resp_index, "next_full");
  ei_encode_binary(resp, &resp_index, rebuff, strlen(rebuff));

  erlcmd_send(resp, resp_index);
  
  exit(EXIT_SUCCESS);
}
