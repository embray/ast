/*
 *  Test Mapping class (PolyMap / LinearApprox).
 *  Converted from the Fortran test testmapping.f.
 *  Direct conversion; no material differences from the Fortran original.
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
   AstPolyMap *pm;
   double coeff[20], fit[6], lbnd[2], ubnd[2];

   /* Coefficients for a 2-in 2-out polynomial:
      out1 = 1.0 + 2.0*x
      out2 = 1.0 + 3.0*y + 3.0*y^2  */
   double c[] = { 1.0, 1, 0, 0,
                  2.0, 1, 1, 0,
                  1.0, 2, 0, 0,
                  3.0, 2, 0, 1,
                  3.0, 1, 0, 2 };

   int i;
   for( i = 0; i < 20; i++ ) coeff[i] = c[i];

   astWatch( status );
   astBegin;

   pm = astPolyMap( 2, 2, 4, coeff, 0, coeff, " " );

   lbnd[0] = -1.0;
   lbnd[1] = -1.0;
   ubnd[0] = 1.0;
   ubnd[1] = 1.0;
   if( astLinearApprox( pm, lbnd, ubnd, 0.001, fit ) ) {
      if( fit[0] != 1.0 || fit[1] != 1.0 ||
          fit[2] != 2.0 || fit[3] != 0.0 ||
          fit[4] != 0.0 || fit[5] != 3.0 ) {
         stopit( status, "Error 0" );
      }
   } else {
      stopit( status, "Error 1" );
   }

   coeff[12] = AST__BAD;
   pm = astPolyMap( 2, 2, 4, coeff, 0, coeff, " " );

   if( astLinearApprox( pm, lbnd, ubnd, 0.001, fit ) ) {
      if( fit[0] != 1.0 || fit[1] != AST__BAD ||
          fit[2] != 2.0 || fit[3] != 0.0 ||
          fit[4] != AST__BAD || fit[5] != AST__BAD ) {
         stopit( status, "Error 2" );
      }
   } else {
      stopit( status, "Error 3" );
   }

   pm = astPolyMap( 2, 2, 5, coeff, 0, coeff, " " );
   if( astLinearApprox( pm, lbnd, ubnd, 0.001, fit ) ) {
      stopit( status, "Error 4" );
   }

   astEnd;

   if( *status == 0 ) {
      printf( " All Mapping tests passed\n" );
   } else {
      printf( "Mapping tests failed\n" );
   }
   return *status;
}
