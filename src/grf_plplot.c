/*
*  Name:
*     grf_plplot.c

*  Purpose:
*     Implement the grf module using the PLPlot graphics system.

*  Description:
*     This file implements the low level graphics functions required
*     by the rest of AST, by calling suitable PLPlot functions.
*
*  Licence:
*     This program is free software: you can redistribute it and/or
*     modify it under the terms of the GNU Lesser General Public
*     License as published by the Free Software Foundation, either
*     version 3 of the License, or (at your option) any later
*     version.
*
*     This program is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU Lesser General Public License for more details.
*
*     You should have received a copy of the GNU Lesser General
*     License along with this program.  If not, see
*     <http://www.gnu.org/licenses/>.
*/

/* Macros */
/* ====== */
#define MXSTRLEN 80              /* String length at which truncation starts */

/* Header files. */
/* ============= */
#include "pointset.h"            /* Defines AST__BAD */
#include "memory.h"              /* Memory allocation facilities */
#include "error.h"               /* Error reporting facilities */
#include "grf.h"                 /* Interface to this module */

/* Error code definitions. */
/* ----------------------- */
#include "ast_err.h"             /* AST error codes */

/* C header files. */
/* --------------- */
#include <float.h>
#include <math.h>
#include <string.h>
#include <plplot.h>              /* PLPlot native C API */

/* Constants. */
/* ========== */
#define R2D 57.29578             /* Radians to degrees factor */

/* Externally visible functions. */
/* ============================= */

int astGBBuf( void ){
   /* PLplot has no explicit begin-buffer function; it manages its own updates. */
   return 1;
}

int astGEBuf( void ){
   /* PLplot has no explicit end-buffer function. */
   return 1;
}

int astGFlush( void ){
   c_plflush();
   return 1;
}

int astGCap( int cap, int value ){
   int result = 0;
   if( cap == GRF__SCALES ) result = 1;
   return result;
}

int astGLine( int n, const float *x, const float *y ){
   int i;
   PLFLT *px, *py;

   if( n > 1 && x && y ){
      px = (PLFLT *) astMalloc( sizeof( PLFLT ) * (size_t) n );
      py = (PLFLT *) astMalloc( sizeof( PLFLT ) * (size_t) n );
      if( astOK ){
         for( i = 0; i < n; i++ ){
            px[ i ] = (PLFLT) x[ i ];
            py[ i ] = (PLFLT) y[ i ];
         }
         c_plline( n, px, py );
         px = (PLFLT *) astFree( (void *) px );
         py = (PLFLT *) astFree( (void *) py );
      }
   }
   return 1;
}

int astGMark( int n, const float *x, const float *y, int type ){
   int i;
   PLFLT *px, *py;

   if( n > 0 && x && y ){
      px = (PLFLT *) astMalloc( sizeof( PLFLT ) * (size_t) n );
      py = (PLFLT *) astMalloc( sizeof( PLFLT ) * (size_t) n );
      if( astOK ){
         for( i = 0; i < n; i++ ){
            px[ i ] = (PLFLT) x[ i ];
            py[ i ] = (PLFLT) y[ i ];
         }
         c_plpoin( n, px, py, type );
         px = (PLFLT *) astFree( (void *) px );
         py = (PLFLT *) astFree( (void *) py );
      }
   }
   return 1;
}

int astGScales( float *alpha, float *beta ){
   PLFLT wx1, wx2, wy1, wy2;
   PLFLT nx1, nx2, ny1, ny2;

   /* Get viewport in world coordinates */
   c_plgvpw( &wx1, &wx2, &wy1, &wy2 );
   /* Get subpage bounds in physical coordinates (mm) */
   c_plgspa( &nx1, &nx2, &ny1, &ny2 );

   if( wx2 != wx1 && wy2 != wy1 &&
       nx2 != nx1 && ny2 != ny1 ) {
      *alpha = (float) ((nx2 - nx1) / (wx2 - wx1));
      *beta  = (float) ((ny2 - ny1) / (wy2 - wy1));
      return 1;
   } else {
      astError( AST__GRFER, "astGScales: The graphics window or viewport has zero size." );
      return 0;
   }
}

int astGText( const char *text, float x, float y, const char *just,
              float upx, float upy ){
   char lj[ 2 ];
   float alpha, beta;
   PLFLT fjust, dx, dy, dx_mm, dy_mm, pl_upx, pl_upy;

   if( text && text[ 0 ] != 0 ){
      if( just ){
         lj[ 0 ] = (just[ 0 ] == 'T' || just[ 0 ] == 'C' || just[ 0 ] == 'B') ? just[ 0 ] : 'C';
         lj[ 1 ] = (just[ 1 ] == 'L' || just[ 1 ] == 'C' || just[ 1 ] == 'R') ? just[ 1 ] : 'C';
      } else {
         lj[ 0 ] = 'C';
         lj[ 1 ] = 'C';
      }

      if( !astGScales( &alpha, &beta ) ) return 0;

      if( alpha < 0.0 ) upx = -upx;
      if( beta < 0.0 ) upy = -upy;

      if( lj[ 1 ] == 'L' ) {
         fjust = 0.0;
      } else if( lj[ 1 ] == 'R' ) {
         fjust = 1.0;
      } else {
         fjust = 0.5;
      }

      /* Let's define the baseline vector dx, dy. The up vector is upx, upy. 
         Baseline vector is perpendicular to the up vector.
         If up vector in mm is (upx*alpha, upy*beta), then baseline vector in mm is
         (upy*beta, -upx*alpha). So in world coordinates: (upy*beta/alpha, -upx). */
      dx_mm = upy * beta;
      dy_mm = -upx * alpha;
      
      /* In world coords */
      dx = dx_mm / alpha;
      dy = dy_mm / beta;

      /* Adjust vertical reference point - PLplot natively uses baseline justification for y */
      /* Approximation for 'T' and 'C' justification */
      if( lj[0] != 'B' ){
         PLFLT def_chr, chr_ht;
         c_plgchr( &def_chr, &chr_ht ); /* chr_ht is in mm */
         
         /* Normalize up vector */
         float uplen = sqrt(upx*upx + upy*upy);
         if( uplen > 0.0 ){
            pl_upx = upx / uplen;
            pl_upy = upy / uplen;
         } else {
            astError( AST__GRFER, "astGText: Zero length up-vector supplied.");
            return 0;
         }
         
         /* Convert chr_ht to world coords height offset roughly */
         PLFLT hu = chr_ht / sqrt(alpha*alpha*pl_upx*pl_upx + beta*beta*pl_upy*pl_upy);
         
         if( lj[ 0 ] == 'T' ){
            x -= pl_upx * hu;
            y -= pl_upy * hu;
         } else if( lj[ 0 ] == 'C' ){
            x -= 0.5 * pl_upx * hu;
            y -= 0.5 * pl_upy * hu;
         }
      }

      c_plptex( (PLFLT) x, (PLFLT) y, dx, dy, fjust, text );
   }
   return 1;
}

int astGTxExt( const char *text, float x, float y, const char *just,
               float upx, float upy, float *xb, float *yb ){
   char lj[ 2 ];
   int i;
   float alpha, beta;
   float uplen, ux, uy, vx, vy, uxu, uyu, uxd, uyd;
   float hu, hd, vdx, vdy, udx, udy, xc, yc;
   PLFLT def_chr, chr_ht;

   for( i = 0; i < 4; i++ ){
      xb[ i ] = 0.0;
      yb[ i ] = 0.0;
   }

   if( text && text[ 0 ] != 0 ){
      if( just ){
         lj[ 0 ] = (just[ 0 ] == 'T' || just[ 0 ] == 'C' || just[ 0 ] == 'B') ? just[ 0 ] : 'C';
         lj[ 1 ] = (just[ 1 ] == 'L' || just[ 1 ] == 'C' || just[ 1 ] == 'R') ? just[ 1 ] : 'C';
      } else {
         lj[ 0 ] = 'C';
         lj[ 1 ] = 'C';
      }

      if( !astGScales( &alpha, &beta ) ) return 0;

      if( alpha < 0.0 ) upx = -upx;
      if( beta < 0.0 ) upy = -upy;

      ux = alpha * upx;
      uy = beta * upy;

      uplen = sqrt( ux*ux + uy*uy );
      if( uplen > 0.0 ){
         ux /= uplen;
         uy /= uplen;
      } else {
         astError( AST__GRFER, "astGTxExt: Zero length up-vector supplied.");
         return 0;
      }

      /* Baseline vector in mm */
      vx = uy;
      vy = -ux;

      c_plgchr( &def_chr, &chr_ht ); 
      /* Approximation for text depth and height in mm */
      hu = chr_ht;
      hd = -0.2 * chr_ht; /* approximate descent */

      uxu = ux * hu;
      uyu = uy * hu;
      uxd = ux * hd;
      uyd = uy * hd;

      /* Width in mm */
      float width_mm = strlen(text) * chr_ht * 0.6f; 
      
      vx *= width_mm;
      vy *= width_mm;

      /* Convert back to world coordinates */
      vx /= alpha;
      vy /= beta;

      uxu /= alpha;
      uyu /= beta;
      uxd /= alpha;
      uyd /= beta;

      xc = x;
      yc = y;

      if( lj[0] == 'B' ) {
         xc += 0.5 * uxu;
         yc += 0.5 * uyu;
      } else if( lj[0] == 'T' ) {
         xc -= 0.5 * uxu;
         yc -= 0.5 * uyu;
      }

      if( lj[1] == 'L' ) {
         xc += 0.5 * vx;
         yc += 0.5 * vy;
      } else if( lj[1] == 'R' ) {
         xc -= 0.5 * vx;
         yc -= 0.5 * vy;
      }

      vdx = 0.5 * vx;
      vdy = 0.5 * vy;
      udx = 0.5 * uxu;
      udy = 0.5 * uyu;

      xb[ 0 ] = xc - vdx - udx + uxd;
      yb[ 0 ] = yc - vdy - udy + uyd;

      xb[ 1 ] = xc + vdx - udx + uxd;
      yb[ 1 ] = yc + vdy - udy + uyd;

      xb[ 2 ] = xc + vdx + udx;
      yb[ 2 ] = yc + vdy + udy;

      xb[ 3 ] = xc - vdx + udx;
      yb[ 3 ] = yc - vdy + udy;
   }
   return 1;
}

int astGQch( float *chv, float *chh ){
   PLFLT def_chr, chr_ht;
   PLFLT wx1, wx2, wy1, wy2;
   PLFLT nx1, nx2, ny1, ny2;

   /* Get character height in mm */
   c_plgchr( &def_chr, &chr_ht );

   /* Get viewport and subpage to calculate mm to world scale */
   c_plgvpw( &wx1, &wx2, &wy1, &wy2 );
   c_plgspa( &nx1, &nx2, &ny1, &ny2 );

   if( nx1 != nx2 ){
      *chv = (float) (chr_ht * (wx2 - wx1) / (nx2 - nx1));
   } else {
      astError( AST__GRFER, "astGQch: The graphics viewport has zero size in X.");
      return 0;
   }

   if( ny1 != ny2 ){
      *chh = (float) (chr_ht * (wy2 - wy1) / (ny2 - ny1));
   } else {
      astError( AST__GRFER, "astGQch: The graphics viewport has zero size in Y.");
      return 0;
   }

   return 1;
}

int astGAttr( int attr, double value, double *old_value, int prim ){
   PLINT ival;

   if( attr == GRF__STYLE ){
      if( value != AST__BAD ){
         ival = (PLINT) ( value + 0.5 );
         if( value < 0.0 ) ival -= 1;
         ival = ( ival - 1 ) % 5;
         ival += ( ival < 0 ) ? 6 : 1;
         c_pllsty( ival );
      }

   } else if( attr == GRF__WIDTH ){
      if( value != AST__BAD ){
         ival = (PLINT) ( value );
         if (ival < 1) ival = 1;
         c_plwidth( ival );
      }

   } else if( attr == GRF__SIZE ){
      if( value != AST__BAD ){
         c_plschr( 0.0, (PLFLT) value );
      }

   } else if( attr == GRF__FONT ){
      if( value != AST__BAD ){
         ival = (PLINT) ( value + 0.5 );
         if( value < 0.0 ) ival -= 1;
         ival = ( ival - 1 ) % 4;
         ival += ( ival < 0 ) ? 5 : 1;
         c_plfont( ival );
      }

   } else if( attr == GRF__COLOUR ){
      if( value != AST__BAD ){
         ival = (PLINT) ( value + 0.5 );
         if( ival < 0 ) ival = 1;
         c_plcol0( ival );
      }

   } else {
      astError( AST__GRFER, "astGAttr: Unknown graphics attribute '%d' requested.", attr );
      return 0;
   }

   /* Suppress unused parameter warnings */
   (void)old_value;
   (void)prim;

   return 1;
}
