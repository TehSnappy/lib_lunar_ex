#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lunar-master/watdefs.h"
#include "lunar-master/mpc_func.h"
#include "lunar-master/lunar.h"
#include "lunar-master/afuncs.h"
#include "lunar-master/date.h"
#include <ei.h>
#include <unistd.h>
#include <sys/select.h>


#define J2000 2451545.0
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define EARTH_MOON_BALANCE 81.300569404492336
#define J2000_OBLIQUITY  (23.4392911 * PI / 180)

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


static double make_ra_dec_string( double *vect, char *buff)
{
  double ra = atan2( vect[1], vect[0]) * 12. / PI;
  const double dist =
          sqrt( vect[0] * vect[0] + vect[1] * vect[1] + vect[2] * vect[2]);
  double dec = asin( vect[2] / dist) * 180. / PI;
  long units;

  if( ra < 0.)
    ra += 24.;

  char resp[2048];
  int resp_index = sizeof(uint16_t);
  units = (long)( ra * 3600. * 10000.);

  ei_encode_version(resp, &resp_index);
  ei_encode_map_header(resp, &resp_index, 2);
  ei_encode_atom(resp, &resp_index, "ra");
  ei_encode_map_header(resp, &resp_index, 3);
  ei_encode_atom(resp, &resp_index, "h");
  ei_encode_long(resp, &resp_index, units / 36000000L);
  ei_encode_atom(resp, &resp_index, "m");
  ei_encode_long(resp, &resp_index, (units / 600000L) % 60L);
  ei_encode_atom(resp, &resp_index, "s");
  ei_encode_long(resp, &resp_index, (units / 10000L) % 60L);  

  sprintf( buff, "%02ldh %02ldm %02ld.%04lds   ",
      units / 36000000L, (units / 600000L) % 60L,
      (units / 10000L) % 60L, units % 10000L);
  buff += strlen( buff);
  int make_neg = 1;

  if( dec < 0.)
    {
    *buff++ = '-';
    dec = -dec;
    make_neg = -1;
    }
  else
    *buff++ = '+';

  units = (long)( dec * 3600. * 1000.);
  sprintf( buff, "%02ld %02ld' %02ld.%03ld\"   ",
      units / 3600000L, (units / 60000L) % 60L,
      (units / 1000L) % 60L, units % 1000L);

  ei_encode_atom(resp, &resp_index, "dec");
  ei_encode_map_header(resp, &resp_index, 3);
  ei_encode_atom(resp, &resp_index, "d");
  ei_encode_long(resp, &resp_index, units / 3600000L * make_neg);
  ei_encode_atom(resp, &resp_index, "m");
  ei_encode_long(resp, &resp_index, (units / 60000L) % 60L);
  ei_encode_atom(resp, &resp_index, "s");
  ei_encode_long(resp, &resp_index, (units / 1000L) % 60L);

  erlcmd_send(resp, resp_index);

  return( dist);
}

#define EARTH_MAJOR_AXIS    6378137.
#define EARTH_MINOR_AXIS    6356752.

static void compute_topocentric_offset( double *earth_vect, const double lat,
          const double lon, const double alt_in_meters, const double jd_ut)
{
  double precess_matrix[9];
  double rho_cos_phi, rho_sin_phi;
  int i;

  lat_alt_to_parallax( lat, alt_in_meters, &rho_cos_phi, &rho_sin_phi,
        EARTH_MAJOR_AXIS, EARTH_MINOR_AXIS);
  rho_cos_phi *= EARTH_MAJOR_AXIS / AU_IN_METERS;
  rho_sin_phi *= EARTH_MAJOR_AXIS / AU_IN_METERS;
  calc_planet_orientation( 3, 0, jd_ut, precess_matrix);
  spin_matrix( precess_matrix, precess_matrix + 3, lon);
  for( i = 0; i < 3; i++)
    earth_vect[i] = (rho_cos_phi * precess_matrix[i]
              + rho_sin_phi * precess_matrix[i + 6]);
}

#define N_PASSES 3

int main(int argc, char** argv) {
{
  double xyz[4], xyz_topo[3], topo_offset[3];
  double jd_utc = 2458386.354977;
  double earth_vect[6];
  double lon = -73.133 * PI / 180.;  /* Project Pluto corporate headquarters is */
  double lat = 40.927 * PI / 180.;  /* roughly at W 69.9, N 44.01, 100 meters alt */
  double alt_in_meters = 0.;
  int i, planet_no, opt;
  char buff[80];
  void *p;

  char priv_path[1024] = "priv";
  char codepath[1024];
  FILE *ifile;
         static const char *planet_names[10] = {
               "Sun", "Mercury", "Venus", "Earth",
               "Mars", "Jupiter", "Saturn", "Uranus",
               "Neptune", "Pluto" };

  while ((opt = getopt(argc, argv, "l:n:d:t:p:c:")) != -1) {
   switch (opt) {
    case 'l': lat = atof(optarg) * PI / 180.; break;
    case 'n': lon = atof(optarg) * PI / 180.; break;
    case 'd': jd_utc = atof(optarg); break;
    case 'p': planet_no = atoi(optarg); break;
    case 'c': strcpy(priv_path, optarg); break;
    default:
       fprintf(stderr, "Usage: %s [-criapdh] [file...]\n", argv[0]);
       exit(EXIT_FAILURE);
    }
  }
  double jd_td = jd_utc + td_minus_utc( jd_utc) / seconds_per_day;
  double jd_ut = jd_td - td_minus_ut( jd_utc) / seconds_per_day;

//  fprintf(stderr, "working with data %f, %f, %f, %d\n\n", lat, lon, jd_utc, planet_no);

  if (planet_no == 10) {
    sprintf(codepath, "%s/elp82.dat", priv_path);
    ifile = fopen( codepath, "rb");

    if( !ifile)
    {
    fprintf(stderr, "Couldn't find 'elp82.dat'\n");
    return( -1);
    }

    compute_elp_xyz( ifile, (jd_td - J2000) / 36525., 0., xyz);
//    fprintf(stderr, "Lunar distance from geocenter: %15.5f km\n", xyz[3]);
          /* Rotate from ecliptic J2000 to equatorial J2000: */
    rotate_vector( xyz, J2000_OBLIQUITY, 0);
    for( i = 0; i < 3; i++)          /* cvt earth-moon vector from km */
      xyz[i] /= AU_IN_KM;           /* into AU */
//    fprintf(stderr, "after Lunar distance from geocenter\n");

    compute_topocentric_offset( topo_offset, lat, lon, alt_in_meters, jd_ut);
    for( i = 0; i < 3; i++)           /* make a topocentric version of the */
      xyz_topo[i] = xyz[i] - topo_offset[i];         /* earth-moon vector */
  
    make_ra_dec_string( xyz_topo, buff);
//    fprintf(stderr, "Topocentric Lunar position: %s\n", buff);
    fclose( ifile);
  }
//  else if( planet_no == 3)
//  {
//      char *vsop_data = load_file_into_memory( codepath, NULL);
//      pdata->ecliptic_lon = calc_vsop_loc( vsop_data, planet_no, 0, t_centuries, 0.) + pi;
//      pdata->ecliptic_lat = -calc_vsop_loc( vsop_data, planet_no, 1, t_centuries, 0.);
//      pdata->r   = calc_vsop_loc( vsop_data, planet_no, 2, t_centuries, 0.);
//  }
  else if( planet_no != 3)
  {
    sprintf(codepath, "%s/ps_1996.dat", priv_path);
    ifile = fopen(codepath, "rb");
    if( !ifile)
      {
      fprintf(stderr,  "Couldn't find 'ps_1996.dat'\n");
      return( -1);
      }
    p = load_ps1996_series( ifile, jd_td, 3);
    if( !p)
      {
      fprintf(stderr, "No planetary data before 1900 or after 2100\n");
      return( -2);
      }
//    fprintf(stderr, "about to get_ps1996_position\n");
    get_ps1996_position( jd_td, p, earth_vect, 0);
//    fprintf(stderr, "about to free load_ps1996_series\n");
    free( p);

    for( i = 0; i < 3; i++)
      earth_vect[i] -= xyz[i] / (EARTH_MOON_BALANCE + 1.);
    for( i = 0; i < 3; i++)
      earth_vect[i] += topo_offset[i];

//    fprintf(stderr, "about to load_ps1996_series\n");

    p = load_ps1996_series( ifile, jd_td, planet_no);
    if( !p)
      fprintf( stderr, "No data for that time");
    else
      {
      double dist = 0., state_vect[6], delta[3];
      int pass;

//      fprintf(stderr, "about to take passes\n");

      for( pass = 0; pass < N_PASSES; pass++)
        {
        get_ps1996_position( jd_td - dist * AU_IN_KM / (SPEED_OF_LIGHT * seconds_per_day),
                   p, state_vect, 0);
        dist = 0.;
        for( i = 0; i < 3; i++)
          {
          delta[i] = state_vect[i] - earth_vect[i];
          dist += delta[i] * delta[i];
          }
        dist = sqrt( dist);
        }
        free( p);
        make_ra_dec_string( delta, buff);
      }
//      fprintf(stderr,  "%-10s %s\n", planet_names[planet_no], buff);
      fclose( ifile);
    }
  }

  exit(EXIT_SUCCESS);
}
