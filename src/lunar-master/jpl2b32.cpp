/* Copyright (C) 2018, Project Pluto

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

/* Very Guide-specific code to convert an ephemeris gathered from JPL's
_Horizons_ system into the .b32 ("binary 32-bit fixed-point integer")
format used in Guide.  Probably of no interest to anyone except me.
See comments below on how to get the _Horizons_ ephemerides in the
first place. */

int main( const int argc, const char **argv)
{
   time_t t0 = time( NULL);
   FILE *ifile = (argc > 1 ? fopen( argv[1], "rb") : NULL);
   FILE *ofile = NULL;
   char buff[200];
   int n_written = 0,  pass;
   const int sat_id = (argc > 1 ? atoi( argv[1]) : 0);
   double jd0 = 0., step_size = 0., jd, max_ordinate = 0;

   if( argc > 1 && !ifile)
      printf( "\nCouldn't open the JPL file '%s'\n", argv[1]);

   if( argc < 2 || !ifile)
      {
      printf( "\nJPL2B32 takes input ephemeri(de)s generated by HORIZONS,\n");
      printf( "and produces .B32 file(s) suitable for use in Guide.  The\n");
      printf( "name of the input ephemeris must be provided as a command-line\n");
      printf(  "argument.  For example:\n");
      printf( "\njpl2b32 9002.txt\n\n");
      printf( "The JPL ephemeris must be in text form (can use the 'download/save'\n");
      printf( "option for this),  with vectors in units of AU.  Also,  the data\n");
      printf( "should be equatorial (not the default ecliptic!) coordinates.\n");
      printf( "You can also select position (xyz) output only,  if you wish,  but\n");
      printf( "you don't have to;  velocities and range/rate/light time will\n");
      printf( "simply be ignored if present.\n");
      exit( -1);
      }

   sprintf( buff, "%d.b32", sat_id);
   ofile = fopen( buff, "wb");
   fwrite( buff, 128, 1, ofile);    /* output dummy 'placeholder' buffer */
   for( pass = 0; pass < 2; pass++)
      {
      n_written = 0;
      fseek( ifile, 0L, SEEK_SET);
      while( fgets( buff, sizeof( buff), ifile))
         if( (jd = atof( buff)) > 2000000. && jd < 3000000. &&
                  strlen( buff) > 54 && !memcmp( buff + 17, " = A.D.", 7)
                  && !memcmp( buff + 37, "00:00:00.0000 (CT)", 18))
            {
            int i;

            if( !fgets( buff, sizeof( buff), ifile))
               {
               printf( "Failed to get data from input file\n");
               return( -2);
               }
            if( !n_written)
               jd0 = jd;
            else if( n_written == 1)
               step_size = jd - jd0;
            for( i = 0; i < 3; i++)
               {
               const double ordinate = atof( buff + i * 23 + 1);

               if( !pass)
                  {
                  if( max_ordinate < fabs( ordinate))
                     max_ordinate = fabs( ordinate);
                  }
               else
                  {
                  const int32_t oval =
                               (int32_t)( ordinate * 1.5e+9 / max_ordinate);

                  fwrite( &oval, sizeof( int32_t), 1, ofile);
                  }
               }
            n_written++;
            }
      }

   fclose( ifile);
   fprintf( ofile, "Ephemeris from JPL Horizons output\n");
   fprintf( ofile, "Created using 'jpl2b32', version %s\n", __DATE__);
   fprintf( ofile, "Ephemeris converted %s", ctime( &t0));
   printf( "JD0: %f   Step size: %f   %ld steps\n",
                               jd0,  step_size, (long)n_written);
                     /* Seek back to start of file & write corrected hdr: */
   fseek( ofile, 0L, SEEK_SET);
   memset( buff, 0, 128);
   sprintf( buff, "%d %d %10.1f %f %ld %d %g %d %d ",
               128, sat_id,               /* JPL ID */
               jd0,  step_size, (long)n_written, 6,
               max_ordinate / 1.5e+9, 32, 0);
   fwrite( buff, 128, 1, ofile);
   fclose( ofile);
   return( 0);
}

/* Following is an example e-mail request to the Horizons server for a
suitable text ephemeris for 2011P1,  a satellite known to Horizons as
major body 904.  For other objects,  you would modify the COMMAND and
CENTER lines,  as well as the START_TIME,  STOP_TIME,  and STEP_SIZE.
And,  of course,  the EMAIL_ADDR.

   Aside from that,  all is as it should be:  vectors are requested
with positions only,  with no light-time corrections,  in equatorial
J2000 coordinates,  in AU (not kilometers).

   After making those modifications,  you would send the result to
horizons@ssd.jpl.nasa.gov, subject line JOB.

!$$SOF (ssd)       JPL/Horizons Execution Control VARLIST
! Full directions are at
! ftp://ssd.jpl.nasa.gov/pub/ssd/horizons_batch_example.long

! EMAIL_ADDR sets e-mail address output is sent to. Enclose
! in quotes. Null assignment uses mailer return address.

 EMAIL_ADDR = 'pluto@projectpluto.com'
 COMMAND    = '904'

! MAKE_EPHEM toggles generation of ephemeris, if possible.
! Values: YES or NO

 MAKE_EPHEM = 'YES'

! TABLE_TYPE selects type of table to generate, if possible.
! Values: OBSERVER, ELEMENTS, VECTORS
! (or unique abbreviation of those values).

 TABLE_TYPE = 'VECTORS'
 CENTER     = '500@999'
 REF_PLANE  = 'FRAME'

! START_TIME specifies ephemeris start time
! (i.e. YYYY-MMM-DD {HH:MM} {UT/TT}) ... where braces "{}"
! denote optional inputs. See program user's guide for
! lists of the numerous ways to specify times. Time zone
! offsets can be set. For example, '1998-JAN-1 10:00 UT-8'
! would produce a table in Pacific Standard Time. 'UT+00:00'
! is the same as 'UT'. Offsets are not applied to TT
! (Terrestrial Time) tables. See TIME_ZONE variable also.

 START_TIME = '2009-JAN-27 00:00 TDB'

! STOP_TIME specifies ephemeris stop time
! (i.e. YYYY-MMM-DD {HH:MM}).

 STOP_TIME  = '2015-JAN-01'
 STEP_SIZE  = '2 day'
 QUANTITIES = '
 REF_SYSTEM = 'J2000'
 OUT_UNITS  = 'AU-D'

! VECT_TABLE = 1 means XYZ only,  no velocity, light-time,
! range, or range-rate.
 VECT_TABLE = '1'

! VECT_CORR selects level of correction: NONE=geometric states
! (which we happen to want); 'LT' = astrometric states, 'LT+S'
! = same with stellar aberration included.
 VECT_CORR = 'NONE'

 CAL_FORMAT = 'CAL'

!$$EOF++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
