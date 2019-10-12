
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "lunar-master/watdefs.h"
#include "lunar-master/afuncs.h"
#include "lunar-master/date.h"

static const long double j2000 = 2451545.0;

static void my_remove_char( char *buff, const char removed)
{
   size_t i, j;

   for( i = j = 0; buff[i]; i++)
      if( buff[i] != removed)
         buff[j++] = buff[i];
   buff[j] = '\0';
}


static void my_show_remainder( char *buff, long double remainder, unsigned precision)
{
   *buff++ = '.';
   assert( remainder >= 0. && remainder < 1.);
   while( precision--)
      {
      unsigned digit;

      remainder *= 10.;
      digit = (unsigned)remainder;
      *buff++ = (char)( '0' + digit);
      assert( digit <= 9);
      remainder -= (double)digit;
      }
   *buff++ = '\0';
}

void DLL_FUNC my_full_ctimel( char *buff, long double t2k, const int format, float tz = 0)
{
   const int precision = (format >> 4) & 0xf, calendar = format & 0xf;
   const int output_format = (format & FULL_CTIME_FORMAT_MASK);
   char *ibuff = buff;   /* keep track of the start of the output */
   int day, month;
   long units, i;
   const int leading_zeroes = (format & FULL_CTIME_LEADING_ZEROES);
   long year, int_t2k, day_of_week;
   long double add_on = 1.;
   long double remains;

   if( output_format == FULL_CTIME_FORMAT_SECONDS)
      units = seconds_per_day;
   else if( output_format == FULL_CTIME_FORMAT_HH_MM)
      units = minutes_per_day;
   else if( output_format == FULL_CTIME_FORMAT_HH)
      units = hours_per_day;
   else                   /* output in days */
      units = 1;
   for( i = precision; i; i--)
      add_on /= 10.;
   if( format & FULL_CTIME_ROUNDING)
      add_on *= 0.5 / (double)units;
   else
      add_on *= 0.05 / seconds_per_day;
   t2k += add_on;

   if( output_format == FULL_CTIME_FORMAT_YEAR)
      {
      char tbuff[40];

      sprintf( tbuff, "%21.16Lf", t2k / 365.25 + 2000.);
      tbuff[precision + 5] = '\0';
      if( !precision)
         tbuff[4] = '\0';
      strcpy( buff, tbuff);
      if( leading_zeroes)
         while( *buff == ' ')
            *buff++ = '0';
      return;
      }
   if( output_format == FULL_CTIME_FORMAT_JD
                     || output_format == FULL_CTIME_FORMAT_MJD)
      {
      char format_str[10];

      sprintf( format_str, "JD %%.%dLf", precision);
      if( output_format == FULL_CTIME_FORMAT_MJD)
         {
         *buff++ = 'M';
         t2k += j2000 - 2400000.5;
         }
      else
         t2k += j2000;
      sprintf( buff, format_str, t2k);
      if( leading_zeroes)
         while( *buff == ' ')
            *buff++ = '0';
      return;
      }

   t2k += .5;
   int_t2k = (long)floorl( t2k);
   day_of_week = (int_t2k + 6) % 7;
   if( day_of_week < 0)    /* keep 0 <= day_of_week < 7: */
      day_of_week += 7;
   if( format & FULL_CTIME_DAY_OF_WEEK_FIRST)
      buff += sprintf( buff, "%s ",
                     set_day_of_week_name( (int)day_of_week, NULL));

   day_to_dmy( int_t2k + 2451545, &day, &month, &year, calendar);

   remains = t2k - (long double)int_t2k;
          /* i.e.,  fractional part of day */

   i = remains * seconds_per_day;
   int hours = i / 3600L;
   int minutes = (i / 60) % 60L;
   int seconds = i % 60L;
   int frnt =  floor(tz);
   int back =  tz * 100 - frnt * 100;

   sprintf( buff, "%4ld-%02d-%02dT%02d:%02d:%02d%+03d:%02d", year, month, day, hours, minutes, seconds, frnt, back);
}

void DLL_FUNC my_full_ctime( char *buff, double jd, const int format, float tz = 0)
{
   my_full_ctimel( buff, (long double)jd - j2000, format, tz);
}

