/*
 *  Test the LutMap class.
 *  Converted from the Fortran test testlutmap.f.
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
   int i;
   AstLutMap *lm;
   double lut1[] = { -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
   double x[7], y[7];

   astWatch( status );
   astBegin;

   /* Linear LUT: output = input for the range -1..8 */
   lm = astLutMap( 10, lut1, -1.0, 1.0, " " );
   x[0] = -2.0; x[1] = -1.0; x[2] = -0.5; x[3] = 3.0;
   x[4] = 7.5;  x[5] = 8.0;  x[6] = 8.5;

   astTran1( lm, 7, x, 1, y );
   for( i = 0; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 1" );
   }

   astTran1( lm, 7, y, 0, x );
   for( i = 0; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 2" );
   }

   /* LUT with duplicate first value (flat start). */
   lut1[0] = lut1[1]; /* lut1[0] = 0 */
   lm = astLutMap( 10, lut1, -1.0, 1.0, " " );
   x[0] = -2.0; x[1] = -1.0; x[2] = -0.5; x[3] = 0.5;
   x[4] = 3.0;  x[5] = 8.0;  x[6] = 8.5;

   astTran1( lm, 7, x, 1, y );
   for( i = 0; i < 3; i++ ) {
      if( y[i] != 0.0 ) stopit( status, "Error 3" );
   }
   for( i = 3; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 4" );
   }

   astTran1( lm, 7, y, 0, x );
   for( i = 0; i < 3; i++ ) {
      if( x[i] != AST__BAD ) stopit( status, "Error 5" );
   }
   for( i = 3; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 6" );
   }

   /* LUT with a BAD value in the middle. */
   lut1[4] = AST__BAD;
   lm = astLutMap( 10, lut1, -1.0, 1.0, " " );
   x[0] = -2.0; x[1] = -1.0; x[2] = -0.5; x[3] = 0.5;
   x[4] = 3.0;  x[5] = 8.0;  x[6] = 8.5;

   astTran1( lm, 7, x, 1, y );
   for( i = 0; i < 3; i++ ) {
      if( y[i] != 0.0 ) stopit( status, "Error 7" );
   }
   if( y[4] != AST__BAD ) stopit( status, "Error 8" );
   y[4] = x[4];
   for( i = 3; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 9" );
   }

   astTran1( lm, 7, y, 0, x );
   for( i = 0; i < 3; i++ ) {
      if( x[i] != AST__BAD ) stopit( status, "Error 10" );
   }
   if( x[4] != AST__BAD ) stopit( status, "Error 11" );
   x[4] = y[4];
   for( i = 3; i < 7; i++ ) {
      if( x[i] != y[i] ) stopit( status, "Error 12" );
   }

   astEnd;
   astFlushMemory( 1 );

   if( *status == 0 ) {
      printf( " All LutMap tests passed\n" );
   } else {
      printf( "LutMap tests failed\n" );
   }
   return *status;
}
