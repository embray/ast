/*
 *  Test the CmpFrame class (NormPoints).
 *  Converted from the Fortran test testcmpframe.f.
 */
#include "ast.h"
#include <stdio.h>

static void stopit( int *status, const char *text ) {
   if( *status != 0 ) return;
   *status = 1;
   printf( "%s\n", text );
}

int main() {
   int status_value = 0;
   int *status = &status_value;
   int perm[3], i;
   AstSkyFrame *sfrm;
   AstFrame *pfrm;
   AstCmpFrame *cfrm;

   double ra[]  = { 6.1, 6.1, 0.04, 0.04 };
   double dec[] = { 0.2, -0.2, -0.2, 0.2 };
   double px[]  = { -100.0, -10.0, 10.0, 100.0 };
   double in[4*3], out[4*3];

   astWatch( status );
   astBegin;

   sfrm = astSkyFrame( " " );
   perm[0] = 2;
   perm[1] = 1;
   astPermAxes( sfrm, perm );

   pfrm = astFrame( 1, "Domain=FPLANE" );

   cfrm = astCmpFrame( pfrm, sfrm, " " );
   perm[0] = 3;
   perm[1] = 1;
   perm[2] = 2;
   astPermAxes( cfrm, perm );

   /* Fill input array: columns are RA, PX, Dec (after permutation).
      Fortran layout in(4,3) with column-major ordering becomes
      a flat array in[npoint*ncoord] with npoint=4, ncoord=3.
      in[i + npoint*coord] = value for point i, coordinate coord. */
   for( i = 0; i < 4; i++ ) {
      in[i + 4*0] = ra[i];
      in[i + 4*1] = px[i];
      in[i + 4*2] = dec[i];
   }

   /* Normalise with `contig=1` (coordinates are contiguous per axis). */
   astNormPoints( cfrm, 4, 3, 4, in, 1, 3, 4, out );

   if( out[0 + 4*0] != ra[0] - 2*AST__DPI ) {
      stopit( status, "Error 1" );
   } else if( out[1 + 4*0] != ra[1] - 2*AST__DPI ) {
      stopit( status, "Error 2" );
   } else if( out[2 + 4*0] != ra[2] ) {
      stopit( status, "Error 3" );
   } else if( out[3 + 4*0] != ra[3] ) {
      stopit( status, "Error 4" );
   } else {
      for( i = 0; i < 4; i++ ) {
         if( out[i + 4*1] != px[i] ) {
            stopit( status, "Error 5" );
         } else if( out[i + 4*2] != dec[i] ) {
            stopit( status, "Error 6" );
         }
      }
   }

   /* Normalise with `contig=0` (do not normalise). */
   astNormPoints( cfrm, 4, 3, 4, in, 0, 3, 4, out );

   for( i = 0; i < 4; i++ ) {
      if( out[i + 4*0] != ra[i] ) {
         stopit( status, "Error 7" );
      } else if( out[i + 4*1] != px[i] ) {
         stopit( status, "Error 8" );
      } else if( out[i + 4*2] != dec[i] ) {
         stopit( status, "Error 9" );
      }
   }

   astEnd;
   astFlushMemory( 1 );

   if( *status == 0 ) {
      printf( " All CmpFrame tests passed\n" );
   } else {
      printf( "CmpFrame tests failed\n" );
   }
   return *status;
}
