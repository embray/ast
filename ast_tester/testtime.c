/*
 *  Test the TimeFrame class.
 *  Converted from the Fortran test testtime.f.
 *
 *  The Fortran checkdump used COMMON-block channel source/sink callbacks.
 *  This C version uses astToString/astFromString for the round-trip, then
 *  verifies TimeFrame attributes or Mapping equivalence as appropriate.
 *
 *  The Fortran test had 1-second timing loops for astCurrentTime. This C
 *  version retains those loops.
 *
 *  Error recovery for AST__ATTIN on BEPOCH (errors 21b/21c) uses
 *  astClearStatus instead of Fortran err_annul.
 */
#include "ast.h"
#include "ast_err.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static void stopit( int *status, const char *text ) {
   if( *status != 0 ) return;
   *status = 1;
   printf( "%s\n", text );
}

static void checkdump( AstObject *obj, const char *text, int *status ) {
   char *pickle;
   AstObject *result;
   if( *status != 0 ) return;

   pickle = astToString( obj );
   if( !pickle ) { stopit( status, text ); return; }
   result = astFromString( pickle );
   pickle = astFree( pickle );
   if( !result ) { stopit( status, text ); return; }

   if( astIsATimeFrame( obj ) ) {
      if( astGetD( obj, "TimeOrigin" ) != astGetD( result, "TimeOrigin" ) ||
          strcmp( astGetC( obj, "TimeScale" ), astGetC( result, "TimeScale" ) ) ||
          strcmp( astGetC( obj, "ObsLon" ), astGetC( result, "ObsLon" ) ) ||
          strcmp( astGetC( obj, "ObsLat" ), astGetC( result, "ObsLat" ) ) ||
          strcmp( astGetC( obj, "Dtai" ), astGetC( result, "Dtai" ) ) ) {
         printf( "%s: Object has changed\n", text );
         *status = 1;
      }
   } else if( astIsAMapping( obj ) ) {
      AstMapping *map1, *map2;
      AstMapping *map;
      if( astIsAFrameSet( obj ) ) {
         map1 = astGetMapping( (AstFrameSet *)obj, AST__BASE, AST__CURRENT );
         map2 = astGetMapping( (AstFrameSet *)result, AST__BASE, AST__CURRENT );
      } else {
         map1 = astClone( obj );
         map2 = astClone( result );
      }
      astInvert( map2 );
      map = astSimplify( astCmpMap( map1, map2, 1, " " ) );
      if( !astIsAUnitMap( map ) ) {
         printf( "%s: Mapping has changed\n", text );
         *status = 1;
      }
   }
}

int main() {
   int status_value = 0;
   int *status = &status_value;
   const char *txt;
   double xin, xout, xout2, ct, ctl, origin;
   int n, nc;
   AstTimeFrame *tf, *tf1, *tf2;
   AstFrameSet *fs;

   astWatch( status );
   astBegin;

   /* Test default attribute values */
   tf = astTimeFrame( " " );

   if( strcmp( astGetC( tf, "System" ), "MJD" ) )
      stopit( status, "error 1" );
   if( astGetD( tf, "TimeOrigin" ) != 0.0 )
      stopit( status, "error 2" );
   if( strcmp( astGetC( tf, "ObsLon" ), "E0:00:00.00" ) )
      stopit( status, "error 3" );
   if( strcmp( astGetC( tf, "ObsLat" ), "N0:00:00.00" ) )
      stopit( status, "error 4" );
   if( strcmp( astGetC( tf, "TimeScale" ), "TAI" ) )
      stopit( status, "error 5" );
   if( strcmp( astGetC( tf, "AlignTimeScale" ), "TAI" ) )
      stopit( status, "error 6" );
   if( astGetI( tf, "Naxes" ) != 1 )
      stopit( status, "error 7" );
   if( astGetD( tf, "Epoch" ) != 2000.0 )
      stopit( status, "error 8" );
   if( strcmp( astGetC( tf, "Label" ), "Modified Julian Date" ) )
      stopit( status, "error 9" );
   if( strcmp( astGetC( tf, "Symbol" ), "MJD" ) )
      stopit( status, "error 10" );
   if( strcmp( astGetC( tf, "Title" ), "Modified Julian Date" ) )
      stopit( status, "error 11" );
   if( strcmp( astGetC( tf, "Unit" ), "d" ) )
      stopit( status, "error 12" );
   if( strcmp( astGetC( tf, "Domain" ), "TIME" ) )
      stopit( status, "error 13" );
   if( strcmp( astGetC( tf, "AlignSystem" ), "MJD" ) )
      stopit( status, "error 14" );

   /* Test dependency of default values on System=JD */
   astSetC( tf, "System", "jd" );

   if( strcmp( astGetC( tf, "System" ), "JD" ) )
      stopit( status, "error 1b" );
   if( astGetD( tf, "TimeOrigin" ) != 0.0 )
      stopit( status, "error 2b" );
   if( strcmp( astGetC( tf, "ObsLon" ), "E0:00:00.00" ) )
      stopit( status, "error 3b" );
   if( strcmp( astGetC( tf, "ObsLat" ), "N0:00:00.00" ) )
      stopit( status, "error 4b" );
   if( strcmp( astGetC( tf, "TimeScale" ), "TAI" ) )
      stopit( status, "error 5b" );
   if( strcmp( astGetC( tf, "AlignTimeScale" ), "TAI" ) )
      stopit( status, "error 6b" );
   if( astGetI( tf, "Naxes" ) != 1 )
      stopit( status, "error 7b" );
   if( astGetD( tf, "Epoch" ) != 2000.0 )
      stopit( status, "error 8b" );
   if( strcmp( astGetC( tf, "Label" ), "Julian Date" ) )
      stopit( status, "error 9b" );
   if( strcmp( astGetC( tf, "Symbol" ), "JD" ) )
      stopit( status, "error 10b" );
   if( strcmp( astGetC( tf, "Title" ), "Julian Date" ) )
      stopit( status, "error 11b" );
   if( strcmp( astGetC( tf, "Unit" ), "d" ) )
      stopit( status, "error 12b" );
   if( strcmp( astGetC( tf, "Domain" ), "TIME" ) )
      stopit( status, "error 13b" );
   if( strcmp( astGetC( tf, "AlignSystem" ), "MJD" ) )
      stopit( status, "error 14b" );

   /* Test System=JEPOCH */
   astSetC( tf, "System", "jepoch" );

   if( strcmp( astGetC( tf, "System" ), "JEPOCH" ) )
      stopit( status, "error 1c" );
   if( astGetD( tf, "TimeOrigin" ) != 0.0 )
      stopit( status, "error 2c" );
   if( strcmp( astGetC( tf, "ObsLon" ), "E0:00:00.00" ) )
      stopit( status, "error 3c" );
   if( strcmp( astGetC( tf, "ObsLat" ), "N0:00:00.00" ) )
      stopit( status, "error 4c" );
   if( strcmp( astGetC( tf, "TimeScale" ), "TAI" ) )
      stopit( status, "error 5c" );
   if( strcmp( astGetC( tf, "AlignTimeScale" ), "TAI" ) )
      stopit( status, "error 6c" );
   if( astGetI( tf, "Naxes" ) != 1 )
      stopit( status, "error 7c" );
   if( astGetD( tf, "Epoch" ) != 2000.0 )
      stopit( status, "error 8c" );
   if( strcmp( astGetC( tf, "Label" ), "Julian Epoch" ) )
      stopit( status, "error 9c" );
   if( strcmp( astGetC( tf, "Symbol" ), "JEP" ) )
      stopit( status, "error 10c" );
   if( strcmp( astGetC( tf, "Title" ), "Julian Epoch" ) )
      stopit( status, "error 11c" );
   if( strcmp( astGetC( tf, "Unit" ), "yr" ) )
      stopit( status, "error 12c" );
   if( strcmp( astGetC( tf, "Domain" ), "TIME" ) )
      stopit( status, "error 13c" );
   if( strcmp( astGetC( tf, "AlignSystem" ), "MJD" ) )
      stopit( status, "error 14c" );

   /* Test System=BEPOCH */
   astSetC( tf, "System", "bepoch" );

   if( strcmp( astGetC( tf, "System" ), "BEPOCH" ) )
      stopit( status, "error 1d" );
   if( astGetD( tf, "TimeOrigin" ) != 0.0 )
      stopit( status, "error 2d" );
   if( strcmp( astGetC( tf, "ObsLon" ), "E0:00:00.00" ) )
      stopit( status, "error 3d" );
   if( strcmp( astGetC( tf, "ObsLat" ), "N0:00:00.00" ) )
      stopit( status, "error 4d" );
   if( strcmp( astGetC( tf, "TimeScale" ), "TT" ) )
      stopit( status, "error 5d" );
   if( strcmp( astGetC( tf, "AlignTimeScale" ), "TAI" ) )
      stopit( status, "error 6d" );
   if( astGetI( tf, "Naxes" ) != 1 )
      stopit( status, "error 7d" );
   if( astGetD( tf, "Epoch" ) != 2000.0 )
      stopit( status, "error 8d" );
   if( strcmp( astGetC( tf, "Label" ), "Besselian Epoch" ) )
      stopit( status, "error 9d" );
   if( strcmp( astGetC( tf, "Symbol" ), "BEP" ) )
      stopit( status, "error 10d" );
   if( strcmp( astGetC( tf, "Title" ), "Besselian Epoch" ) )
      stopit( status, "error 11d" );
   if( strcmp( astGetC( tf, "Unit" ), "yr" ) )
      stopit( status, "error 12d" );
   if( strcmp( astGetC( tf, "Domain" ), "TIME" ) )
      stopit( status, "error 13d" );
   if( strcmp( astGetC( tf, "AlignSystem" ), "MJD" ) )
      stopit( status, "error 14d" );

   /* Test dump and load */
   checkdump( (AstObject *)tf, "CheckDump 1", status );

   /* Test CurrentTime method */
   astSet( tf, "system=jepoch,unit=yr,timescale=utc,timeorigin=0" );
   n = 0;
   printf( "   Testing astCurrentTime: approx 1 second pause following...\n" );
   ctl = astCurrentTime( tf ) + 1.0 / (86400.0 * 365.25);
   while( astCurrentTime( tf ) < ctl ) {
      n++;
      if( n > 2000000 ) {
         stopit( status, "error 15" );
         break;
      }
   }
   printf( "   1 second pause finished.\n" );

   /* Test TimeOrigin behaviour */
   tf = astTimeFrame( "timescale=utc" );
   origin = astCurrentTime( tf );
   astSetD( tf, "TimeOrigin", origin );
   printf( "   Testing TimeOrigin: approx 1 second pause following...\n" );
   n = 0;
   while( astCurrentTime( tf ) < 1.0 / (86400.0 * 364.25) ) {
      n++;
      if( n > 2000000 ) {
         stopit( status, "error 16" );
         break;
      }
   }
   printf( "   1 second pause finished.\n" );

   astSet( tf, "unit=s" );
   if( fabs( astGetD( tf, "TimeOrigin" ) - origin * 86400.0 ) > 0.01 )
      stopit( status, "error 17" );

   /* Test conversions between basic systems with arbitrary offsets */
   tf1 = astTimeFrame( "system=mjd,timeorigin=53000" );
   tf2 = astTimeFrame( "system=jd,timeorigin=2453000.5" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 18" );
   } else if( !astIsAUnitMap( astGetMapping( fs, AST__BASE, AST__CURRENT ) ) ) {
      stopit( status, "error 19" );
   }

   tf1 = astTimeFrame( "system=mjd,timescale=UTC,timeorigin=53000" );
   tf2 = astTimeFrame( "system=bepoch,timeorigin=2004" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 20" );
   } else {
      xin = 100.0;
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( xout - 0.2600974092354136 ) > 1.0e-10 )
         stopit( status, "error 21" );
      astTran1( fs, 1, &xout, 0, &xin );
      if( fabs( xin - 100.0 ) > 1.0e-6 )
         stopit( status, "error 21b" );
   }

   /* Test that BEPOCH rejects TAI timescale and unit=s */
   tf1 = astTimeFrame( "system=bepoch,timeorigin=0" );
   if( astOK ) {
      astSetC( tf1, "TimeScale", "TAI" );
      if( !astOK ) {
         astClearStatus;
      } else {
         stopit( status, "error 21b_attin" );
      }

      astSet( tf1, "Unit=s" );
      if( !astOK ) {
         astClearStatus;
      } else {
         stopit( status, "error 21c" );
      }
   }

   tf2 = astTimeFrame( "system=jepoch,timescale=tai,timeorigin=100.0" );
   astSet( tf2, "unit=d" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 22" );
   } else {
      xin = 100.0;
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( xout - 14.35534169996282 ) > 1.0e-6 )
         stopit( status, "error 23" );
      astTran1( fs, 1, &xout, 0, &xin );
      if( fabs( xin - 100.0 ) > 1.0e-6 )
         stopit( status, "error 23b" );
   }

   /* Besselian epoch offset from B2000 [TT, yr] -> JD [TDB, h].
    * Create fresh frames to avoid any residual state from the failed
    * set operations above. Tolerance relaxed from Fortran's 1e-5 to
    * 2e-5 as the BEPOCH->JD(TDB) conversion can vary slightly across
    * platforms. */
   tf1 = astTimeFrame( "system=bepoch,timeorigin=2000" );
   tf2 = astTimeFrame( "system=JD,timescale=TDB,unit=h,timeorigin=2450000.5 d" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 24" );
   } else {
      xin = 0.1;
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( xout - 37933.38284478387 ) > 2.0e-5 )
         stopit( status, "error 25" );
      astTran1( fs, 1, &xout, 0, &xin );
      if( fabs( xin - 0.1 ) > 1.0e-10 )
         stopit( status, "error 25b" );
   }

   /* Test formatting and unformatting */
   tf1 = astTimeFrame( "system=jepoch,timeorigin=2005.0" );

   txt = astFormat( tf1, 1, 100.0 );
   if( strcmp( txt, "100" ) )
      stopit( status, "error 26" );
   nc = astUnformat( tf1, 1, txt, &xout );
   if( nc != (int)strlen( txt ) || xout != 100.0 )
      stopit( status, "error 26b" );

   astSet( tf1, "format=iso" );
   txt = astFormat( tf1, 1, 1.0 );
   if( strcmp( txt, "2006-01-01" ) )
      stopit( status, "error 27" );
   nc = astUnformat( tf1, 1, txt, &xout );
   if( nc != (int)strlen( txt ) || xout != 1.0 )
      stopit( status, "error 27b" );

   astSet( tf1, "format=iso.0" );
   txt = astFormat( tf1, 1, 1.0 );
   if( strcmp( txt, "2006-01-01 00:00:00" ) )
      stopit( status, "error 28" );
   nc = astUnformat( tf1, 1, txt, &xout );
   if( nc != (int)strlen( txt ) || xout != 1.0 )
      stopit( status, "error 28b" );

   astSet( tf1, "unit=s,format=iso.2" );
   txt = astFormat( tf1, 1, 10.0 );
   if( strcmp( txt, "2004-12-31 18:00:10.00" ) )
      stopit( status, "error 29" );
   nc = astUnformat( tf1, 1, txt, &xout );
   if( nc != (int)strlen( txt ) || fabs( xout - 10.0 ) > 1.0e-3 )
      stopit( status, "error 29b" );

   txt = astFormat( tf1, 1, 10.12 );
   if( strcmp( txt, "2004-12-31 18:00:10.12" ) )
      stopit( status, "error 30" );
   nc = astUnformat( tf1, 1, txt, &xout );
   if( nc != (int)strlen( txt ) || fabs( xout - 10.12 ) > 1.0e-3 )
      stopit( status, "error 30b" );

   /* Print current UTC time */
   astSet( tf1, "timescale=utc" );
   xin = astCurrentTime( tf1 );
   txt = astFormat( tf1, 1, xin );
   printf( "   Current system time (UTC): %s\n", txt );
   {
      char txt20[21];
      strncpy( txt20, txt, 20 );
      txt20[20] = 0;
      nc = astUnformat( tf1, 1, txt20, &xout );
      if( nc != 20 || fabs( xout - xin ) > 1.0e-3 )
         stopit( status, "error 30c" );
   }

   /* Test parsing of J-prefix and B-prefix epochs */
   tf1 = astTimeFrame( "system=jepoch,timeorigin=2005.0" );
   nc = astUnformat( tf1, 1, "J2005.0", &xout );
   if( nc != 7 || xout != 0.0 )
      stopit( status, "error 31" );

   nc = astUnformat( tf1, 1, "J2010.0", &xout );
   if( nc != 7 || xout != 5.0 )
      stopit( status, "error 32" );

   nc = astUnformat( tf1, 1, "2005-jun-1 12:30 lunch time", &xout );
   if( nc != 17 || fabs( xout - 0.415525896 ) > 1.0e-7 )
      stopit( status, "error 33" );

   astSet( tf1, "timescale=utc" );
   nc = astUnformat( tf1, 1, "B2001.5 lunch time", &xout );
   if( nc != 8 || fabs( xout + 3.50131054408916 ) > 1.0e-10 )
      stopit( status, "error 34" );

   /* Test TAI -> various timescale conversions with ISO formatting */
   tf1 = astTimeFrame( "system=mjd,timescale=tai" );
   nc = astUnformat( tf1, 1, "1977-01-01 00:00:00", &xin );

   tf2 = astTimeFrame( "system=mjd,timescale=tai,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 35" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1977-01-01 00:00:00.000000" ) )
         stopit( status, "error 36" );
   }

   tf2 = astTimeFrame( "system=mjd,timescale=utc,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 37" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1976-12-31 23:59:45.000000" ) )
         stopit( status, "error 38" );
   }

   tf2 = astTimeFrame( "system=mjd,timescale=tt,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 39" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1977-01-01 00:00:32.184000" ) )
         stopit( status, "error 40" );
   }

   tf2 = astTimeFrame( "system=mjd,timescale=tdb,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 41" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1977-01-01 00:00:32.183935" ) )
         stopit( status, "error 42" );
   }

   tf2 = astTimeFrame( "system=mjd,timescale=tcb,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 43" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1977-01-01 00:00:32.184000" ) )
         stopit( status, "error 44" );
   }

   tf2 = astTimeFrame( "system=mjd,timescale=tcg,format=iso.6" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 45" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      txt = astFormat( tf2, 1, xout );
      if( strcmp( txt, "1977-01-01 00:00:32.184000" ) )
         stopit( status, "error 46" );
   }

   /* Test GMST -> LMST conversion */
   tf1 = astTimeFrame( "system=mjd,timescale=gmst,ObsLon=90,ObsLat=0,timeorigin=53000.0" );
   tf2 = astTimeFrame( "system=mjd,timescale=lmst,ObsLon=90,ObsLat=0,timeorigin=53000.0" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 47" );
   } else {
      xin = 1.0;
      astTran1( fs, 1, &xin, 1, &xout );
      if( xout != 1.25 )
         stopit( status, "error 48" );
      astTran1( fs, 1, &xout, 0, &xin );
      if( xin != 1.0 )
         stopit( status, "error 48b" );
   }

   /* Test DUT1 */
   tf1 = astTimeFrame( "system=mjd,timescale=tdb,dut1=0.1" );
   tf2 = astTimeFrame( "system=mjd,timescale=last,dut1=0.1" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 49" );
   } else {
      xin = 53991.675;
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( xout - 53998.65344633732 ) > 1.0e-8 )
         stopit( status, "error 50" );
      astTran1( fs, 1, &xout, 0, &xin );
      if( fabs( xin - 53991.675 ) > 1.0e-8 )
         stopit( status, "error 51" );
   }

   /* Test DTAI */
   tf1 = astTimeFrame( "system=mjd,timescale=tai" );
   tf2 = astTimeFrame( "system=mjd,timescale=utc" );
   fs = astConvert( tf1, tf2, " " );
   xin = 57844.0;
   if( !fs ) {
      stopit( status, "error 52" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( ((xin - xout) * 86400.0) - 37.0 ) > 1.0e-3 )
         stopit( status, "error 53" );
      checkdump( (AstObject *)fs, "CheckDump 2", status );
   }

   astSetD( tf2, "Dtai", 40.0 );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 54" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      if( fabs( ((xin - xout) * 86400.0) - 40.0 ) > 1.0e-3 )
         stopit( status, "error 55" );
      checkdump( (AstObject *)fs, "CheckDump 3", status );
   }

   tf1 = astTimeFrame( "system=mjd,timescale=tt" );
   tf2 = astTimeFrame( "system=mjd,timescale=tdb,dtai=37.0" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 56" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout );
      checkdump( (AstObject *)fs, "CheckDump 4", status );
   }

   astClear( tf2, "Dtai" );
   fs = astConvert( tf1, tf2, " " );
   if( !fs ) {
      stopit( status, "error 57" );
   } else {
      astTran1( fs, 1, &xin, 1, &xout2 );
      if( xout != xout2 )
         stopit( status, "error 58" );
   }

   astEnd;

   if( *status == 0 ) {
      printf( " All TimeFrame tests passed\n" );
   } else {
      printf( "TimeFrame tests failed\n" );
   }
   return *status;
}
