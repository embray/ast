#include "ast.h"
#include "mers.h"
#include "sae_par.h"
#include <math.h>
#include <string.h>


void stopit( int i, int *status );
void test1( AstObject *obj, const char *text, int *status );
void test2( AstObject *obj, const char *text, int *status );
void test3( AstObject *obj, AstObject *obj2, const char *text, int *status );

int chrMatch( const char *a, const char *b ){
   int result = 0;
   if( a && b ) result = !strcmp( a, b );
   return result;
}

int main(){

   int status_value;
   int *status = &status_value;
   AstObject *obj;
   AstYamlChan *ch;
   AstObject *obj2;
   AstChannel *ch2;

   status_value = SAI__OK;

   astBegin;

//     call ast_watchmemory( 8200 );

   ch = astYamlChan( NULL, NULL, " " );

   if( !chrMatch( astGetC( ch, "YAMLENCODING" ), "ASDF" ) ) stopit( -1, status );

   if( astTest( ch, "YAMLENCODING" ) ) stopit( -2, status );

   astSetC( ch, "YamlEncoding", "ASDF" );

   if( !astTest( ch, "YAMLENCODING" ) ) stopit( -3, status );

   if( !chrMatch( astGetC( ch, "YAMLENCODING" ), "ASDF" ) ) stopit( -4, status );

   astClear( ch, "YamlEncoding" );

   if( astTest( ch, "YAMLENCODING" ) ) stopit( -5, status );

   if( !chrMatch(astGetC( ch, "YAMLENCODING" ),"ASDF") ) stopit( -6, status );

   astSet( ch, "SourceFile=imaging_wcs.asdf,SinkFile=yamltest.asdf" );

   obj = astRead( ch );
   test1( obj, "Read tests failed for imaging_wcs.asdf", status );

   if( astWrite( ch, obj ) != 1 ) stopit( 13, status );

   astClear( ch, "SourceFile,SinkFile" );
   astSet( ch, "SourceFile=yamltest.asdf" );
   astClear( ch, "YamlEncoding" );

   obj2 = astRead( ch );
   if( !chrMatch( astGetC( ch, "YAMLENCODING" ), "ASDF" ) ) stopit( 131, status );
   test1( obj, "Read tests failed for yamltest.asdf", status );

   ch2 = astChannel( NULL, NULL, " " );
   astSet( ch2, "SourceFile=tanSipWcs.txt" );
   obj = astRead( ch2 );
   test2( obj, "Read tests failed for tanSipWcs.txt", status );

   astClear( ch, "SourceFile,SinkFile" );
   astSet( ch, "SinkFile=tanSipWcs.asdf" );
   if( astWrite( ch, obj ) != 1 ) stopit( 14, status );

   astClear( ch, "SourceFile,SinkFile" );
   astSet( ch, "SourceFile=tanSipWcs.asdf" );
   obj2 = astRead( ch );

   test2( obj2, "Read tests failed for tanSipWcs.asdf", status );

   astSet( ch2, "SourceFile=lsst_wcs.txt" );
   obj = astRead( ch2 );

   astSet( ch, "SinkFile=lsst_wcs.asdf" );
   if( astWrite( ch, obj ) != 1 ) stopit( 15, status );

   astClear( ch, "SinkFile" );
   astSet( ch, "SourceFile=lsst_wcs.asdf" );
   obj2 = astRead( ch );

   if( !obj2 ) stopit( 16, status );

   test3( obj, obj2, "Tests failed for lsst_wcs.txt", status );

/*Test NATIVE encoding. */
   ch = astYamlChan( NULL, NULL, "YamlEncoding=NATIVE " );

   if( !chrMatch( astGetC( ch, "YAMLENCODING" ), "NATIVE" ) ) stopit( 17, status );

   if( !astTest( ch, "YAMLENCODING" ) ) stopit( 18, status );

   astSet( ch, "SinkFile=nativetest.yaml" );

   if( astWrite( ch, obj ) != 1 ) stopit( 19, status );

   astClear( ch, "YamlEncoding" );
   astClear( ch, "SinkFile" );
   astSet( ch, "SourceFile=nativetest.yaml" );

   obj2 = astRead( ch );

   if( !obj2 ) stopit( 20, status );

   if( !astEqual( obj2, obj ) ) stopit( 21, status );

   if( !chrMatch( astGetC( ch, "YAMLENCODING" ), "NATIVE") ) stopit( 22, status );

   astEnd;
//   astActivememory( " " );
//   astFlushmemory( 1 );

   if( *status == SAI__OK ) {
      printf( " All YamlChan tests passed\n" );
   } else {
      printf( "YamlChan tests failed\n" );
   }

}



void stopit( int i, int *status ){
   if( *status == SAI__OK ) {
      printf("Error %d\n", i );
      *status = SAI__ERROR;
   }
}



void test1( AstObject *obj, const char *text, int *status ){
   double xout[ 6 ];
   double yout[ 6 ];
   double xin[ 6 ] = {-0.25, -2.0, -1.0,  0.1, 1.5, 1.0 };
   double yin[ 6 ] = { 0.0, 0.0, -2.5, -0.2, 2.5, 2.5};

   if( *status != SAI__OK ) return;

   astTran2( obj, 6, xin, yin, 1, xout, yout );

   if( fabs( xout[ 0 ] - 0.0964300052 ) > 1.0E-10 ) {
      stopit( 1, status );
   } else if( fabs( xout[ 1 ] - 0.0964301088 ) > 1.0E-10 ) {
      stopit( 2, status );
   } else if( fabs( xout[ 2 ] - 0.0964301532 ) > 1.0E-10 ) {
      stopit( 3, status );
   } else if( fabs( xout[ 3 ] - 0.0964299927 ) > 1.0E-10 ) {
      stopit( 4, status );
   } else if( fabs( xout[ 4 ] - 0.0964297983 ) > 1.0E-10 ) {
      stopit( 5, status );
   } else if( fabs( xout[ 5 ] - 0.0964298276 ) > 1.0E-10 ) {
      stopit( 6, status );
   } else if( fabs( yout[ 0 ] + 1.2575438634 ) > 1.0E-10 ) {
      stopit( 7, status );
   } else if( fabs( yout[ 1 ] + 1.25754399404 ) > 1.0E-10 ) {
      stopit( 8, status );
   } else if( fabs( yout[ 2 ] + 1.25754387354 ) > 1.0E-10 ) {
      stopit( 9, status );
   } else if( fabs( yout[ 3 ] + 1.25754383357 ) > 1.0E-10 ) {
      stopit( 10, status );
   } else if( fabs( yout[ 4 ] + 1.25754377796 ) > 1.0E-10 ) {
      stopit( 11, status );
   } else if( fabs( yout[ 5 ] + 1.25754381562 ) > 1.0E-10 ) {
      stopit( 12, status );
   }

   if( *status != SAI__OK ) printf( "%s\n", text );

}



void test2( AstObject *obj, const char *text, int *status ){
   int i;
   double xout[ 6 ];
   double xin[ 6 ] = {-25.0, -200.0, -100.0,  10.0, 150.0, 100.0 };
   double xrec[ 6 ];
   double xoutT[ 6 ] = { 0.7408749950008373, 0.7397105172925298, 0.7404083214098485, 0.7411104249728614, 0.7420090246546116, 0.741675898352057 };
   double yout[ 6 ];
   double yin[ 6 ] = {0.0,  0.0, -250.0, -20.0, 250.0, 250.0 };
   double yrec[ 6 ];
   double youtT[ 6 ] = { 0.7658201704375505, 0.7658040776691578, 0.7646142651379874, 0.7657273765608887, 0.7670347808637439, 0.7670304352978341 };

   if( *status != SAI__OK ) return;

   astTran2( obj, 6, xin, yin, 1, xout, yout );
   for( i = 0; i < 6; i++ ){
      if( fabs( xout[ i ] - xoutT[ i ] ) > 1.0E-12 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", xout[ i ], xoutT[ i ], fabs( xout[ i ] - xoutT[ i ] ));
         stopit( i, status );
      } else if( fabs( yout[ i ] - youtT[ i ] ) > 1.0E-12 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", yout[ i ], youtT[ i ], fabs( yout[ i ] - youtT[ i ] ));
         stopit( i + 6, status );
      }
   }


   astTran2( obj, 6, xout, yout, 0, xrec, yrec );
   for( i = 0; i < 6; i++ ){
      if( fabs( xin[ i ] - xrec[ i ] ) > 1.0E-8 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", xin[ i ], xrec[ i ], fabs( xin[ i ] - xrec[ i ] ));
         stopit( i + 12, status );
      } else if( fabs( yin[ i ] - yrec[ i ] ) > 1.0E-8 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", yin[ i ], yrec[ i ], fabs( yin[ i ] - yrec[ i ] ));
         stopit( i + 18, status );
      }
   }

   if( *status != SAI__OK ) printf( "%s\n", text );

}


void test3( AstObject *obj, AstObject *obj2, const char *text, int *status ){

   int i;
   double xout[ 6 ];
   double xout2[ 6 ];
   double xin[ 6 ] = {-25.0, -200.0, -100.0,  10.0, 150.0, 100.0};
   double xrec[ 6 ];
   double xrec2[ 6 ];
   double yout[ 6 ];
   double yout2[ 6 ];
   double yin[ 6 ] = {0.0,  0.0, -250.0, -20.0, 250.0, 250.0};
   double yrec[ 6 ];
   double yrec2[ 6 ];

   if( *status != SAI__OK ) return;

   astTran2( obj, 6, xin, yin, 1, xout, yout );
   astTran2( obj2, 6, xin, yin, 1, xout2, yout2 );

   for( i = 0; i < 6; i++ ){
      if( fabs( xout[ i ] - xout2[ i ] ) > 1.0E-12 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", xout[ i ], xout2[ i ], fabs( xout[ i ] - xout2[ i ] ));
         stopit( i, status );
      } else if( fabs( yout[ i ] - yout2[ i ] ) > 1.0E-12 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", yout[ i ], yout2[ i ], fabs( yout[ i ] - yout2[ i ] ));
         stopit( i + 6, status );
      }
   }


   astTran2( obj, 6, xout, yout, 0, xrec, yrec );
   astTran2( obj, 6, xout2, yout2, 0, xrec2, yrec2 );

   for( i = 0; i < 6; i++ ){
      if( fabs( xrec[ i ] - xrec2[ i ] ) > 1.0E-8 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", xrec[ i ], xrec2[ i ], fabs( xrec[ i ] - xrec2[ i ] ) );
         stopit( i + 12, status );
      } else if( fabs( yrec[ i ] - yrec2[ i ] ) > 1.0E-8 ) {
         if( *status == SAI__OK ) printf("%g %g %g\n", yrec[ i ], yrec2[ i ], fabs( yrec[ i ] - yrec2[ i ] ) );
         stopit( i + 18, status );
      }
   }


   if( *status != SAI__OK ) printf( "%s\n", text );

}




