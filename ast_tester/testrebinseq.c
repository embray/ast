#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ast.h"

static void tests(int ndim, int spread, double *params, int *status);
static void test1(int ndim, int spread, double *params, int *status);
static void test2(int ndim, int spread, double *params, int *status);
static void test3(int ndim, int spread, double *params, int *status);
static void test4(int ndim, int spread, double *params, int *status);
static void test5(int ndim, int spread, double *params, int *status);
static void test6(int ndim, int spread, double *params, int *status);
static void test7(int ndim, int spread, double *params, int *status);
static void test8(int ndim, int spread, double *params, int *status);
static void test9(int ndim, int spread, double *params, int *status);
static void test10(int ndim, int spread, double *params, int *status);
static void test11(int ndim, int spread, double *params, int *status);
static void test12(int ndim, int spread, double *params, int *status);
static void addnoise(int N, double *array, double sigma, int *status);

int main(void) {
   int status_val = 0;
   int *status = &status_val;
   double params[2];
   astWatch(status);
   astBegin;

*status = 0;

astBegin;

params[1-1] = 1.5;
params[2-1] = 1;

tests( 1, AST__GAUSS, params, status );
tests( 2, AST__GAUSS, params, status );
tests( 3, AST__GAUSS, params, status );
tests( 1, AST__NEAREST, params, status );
tests( 2, AST__NEAREST, params, status );
tests( 3, AST__NEAREST, params, status );
tests( 1, AST__LINEAR, params, status );
tests( 2, AST__LINEAR, params, status );
tests( 3, AST__LINEAR, params, status );

astEnd;

/* astFlushMemory(1); */

if ( *status == 0 ) {
printf("%s\n", "All AST_REBINSEQ tests passed");
} else {
printf("%s\n", "AST_REBINSEQ tests failed");
}

   return 0;
}

static void tests(int ndim, int spread, double *params, int *status) {


if ( *status != 0 ) return;

test1( ndim, spread, params, status );
test2( ndim, spread, params, status );
test3( ndim, spread, params, status );
test4( ndim, spread, params, status );
test5( ndim, spread, params, status );
test6( ndim, spread, params, status );
test7( ndim, spread, params, status );
test8( ndim, spread, params, status );
test9( ndim, spread, params, status );
test10( ndim, spread, params, status );
test11( ndim, spread, params, status );
test12( ndim, spread, params, status );
test12( ndim, spread, params, status );

if ( *status != 0 ) {

if ( spread == AST__GAUSS ) {
} else if ( spread == AST__NEAREST ) {
} else if ( spread == AST__LINEAR ) {
}
*status = 1; printf("Error: %s\n", " ");
}

}




static void addnoise(int n, double *array, double sigma, int *status) {
   int bufsize = 100, nused = 0, i = 0;
   AstMapping *mm = NULL;
   double noise[100]={0}, junk[100] = {0};
   char fwd[80], inv[80];
   const char *fwd_ptr[1], *inv_ptr[1];





if ( *status != 0 ) return;

/* handled */
/* iat = 12; */
snprintf(fwd, sizeof(fwd), "Y=Gauss(0.0,%g)", sigma);
strcpy(inv, "X");
fwd_ptr[0] = fwd;
inv_ptr[0] = inv;
/* handled */

mm = (AstMapping*)astMathMap(1, 1, 1, fwd_ptr, 1, inv_ptr, "");

nused = bufsize;
for (i = 1; i <= n; i++) {
if ( nused == bufsize ) {
astTran1(mm, bufsize, junk, 1, noise);
nused = 0;
}
nused = nused + 1;
array[i-1] = array[i-1] + noise[nused-1];
}

astAnnul(mm);

}






static void test1(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double weights_r[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0};
   int in_i[20000]={0};





if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
lbnd_in[3-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;
ubnd_in[3-1] = 1;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND;

map = (AstMapping*)astUnitMap( ndim, "");
astRebinSeqF(map, 0.0, ndim, lbnd_in, ubnd_in, (const float *)in_r, (const float *)in_r, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (float *)out_r, (float *)out_r, (double *)weights_r, &nused);

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {
if ( fabs( out_r[(j-1)*nx + i - 1] - 1.0 ) > 1.0E-6 && *status == 0 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}
}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}



static void test2(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0};
   int in_i[20000]={0};





if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT;

map = (AstMapping*)astUnitMap( ndim, "");

for (i = 1; i <= 3; i++) {
if ( i == 3 ) flags = AST__REBINEND;
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)in_d, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)out_d, (double *)weights_d, &nused);
}

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {
if ( fabs( out_r[(j-1)*nx + i - 1] - 1.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}
}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}





static void test3(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double weights_r[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0};
   int in_i[20000]={0}, out_i[20000]={0};




if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__NONORM;

map = (AstMapping*)astUnitMap( ndim, "");

for (i = 1; i <= 3; i++) {
astRebinSeqI(map, 0.0, ndim, lbnd_in, ubnd_in, (const int *)in_i, (const int *)in_i, spread, params, flags, 0.01, 50, -2147483648, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (int *)out_i, (int *)out_i, (double *)weights_r, &nused);
flags = AST__NONORM;
}

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {
if ( out_r[(j-1)*nx + i - 1] != 3 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}
}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}






static void test4(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int lboxg[3]={0}, uboxg[3]={0};
   int64_t nused=0;
   AstMapping *map = NULL;
   double sum = 0.0;
   double weights_r[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0};
   int in_i[20000]={0};
   double ina[3]={0}, inb[3]={0}, outa[3]={0}, outb[3]={0}, answer=0;




if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__CONSERVEFLUX;

ina[1-1] = lbnd_in[1-1];
ina[2-1] = lbnd_in[2-1];
ina[3-1] = lbnd_in[3-1];
inb[1-1] = ubnd_in[1-1];
inb[2-1] = ubnd_in[2-1];
inb[3-1] = ubnd_in[3-1] + 1.0;

outa[1-1] = 0.75*lbnd_out[1-1] + 0.25*ubnd_out[1-1];
outa[2-1] = 0.75*lbnd_out[2-1] + 0.25*ubnd_out[2-1];
outa[3-1] = ina[3-1];
outb[1-1] = 0.25*lbnd_out[1-1] + 0.75*ubnd_out[1-1];
outb[2-1] = 0.25*lbnd_out[2-1] + 0.75*ubnd_out[2-1];
outb[3-1] = inb[3-1];

map = (AstMapping*)astWinMap( ndim, ina, inb, outa, outb, "");
astRebinSeqF(map, 0.0, ndim, lbnd_in, ubnd_in, (const float *)in_r, (const float *)in_r, spread, params, flags, 0.01, 1000, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (float *)out_r, (float *)out_r, (double *)weights_r, &nused);

lboxg[1-1] = 2147483647;
lboxg[2-1] = 2147483647;
uboxg[1-1] = -2147483648;
uboxg[2-1] = -2147483648;

sum = 0.0;

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {

if ( out_r[(j-1)*nx + i - 1] != AST__BAD ) {
sum = sum + out_r[(j-1)*nx + i - 1];
if ( i < lboxg[1-1] ) {
lboxg[1-1] = i;
} else if ( i > uboxg[1-1] ) {
uboxg[1-1] = i;
}
if ( j < lboxg[2-1] ) {
lboxg[2-1] = j;
} else if ( j > uboxg[2-1] ) {
uboxg[2-1] = j;
}
}
}
}

if ( ndim == 1 ) {

if ( ( ( spread == AST__GAUSS ) && ( lboxg[1-1] != 24 || uboxg[1-1] != 77 )) || ( ( spread == AST__NEAREST ) && ( lboxg[1-1] != 26 || uboxg[1-1] != 75 )) || ( ( spread == AST__LINEAR ) && ( lboxg[1-1] != 25 || uboxg[1-1] != 76 ) ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
printf("Write: %d %d\n", lboxg[0], uboxg[0]);
}

if ( spread == AST__NEAREST ) {
answer = 100.0;
} else if ( spread == AST__GAUSS ) {
answer = 108.0;
} else if ( spread == AST__LINEAR ) {
answer = 104.0;
} else {
answer = -1;
}

} else {

if ( ( ( spread == AST__GAUSS ) && ( lboxg[1-1] != 24 || lboxg[2-1] != 49 || uboxg[1-1] != 77 || uboxg[2-1] != 152 ) ) || ( ( spread == AST__NEAREST ) && ( lboxg[1-1] != 26 || lboxg[2-1] != 51 || uboxg[1-1] != 75 || uboxg[2-1] != 150 ) ) || ( ( spread == AST__LINEAR ) && ( lboxg[1-1] != 25 || lboxg[2-1] != 50 || uboxg[1-1] != 76 || uboxg[2-1] != 151 ) ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
printf("Write: %d %d\n", lboxg[0], uboxg[0]);
}

if ( spread == AST__NEAREST ) {
answer = 20000.0;
} else if ( spread == AST__GAUSS ) {
answer = 22464.0;
} else if ( spread == AST__LINEAR ) {
answer = 21216.0;
} else {
answer = -1;
}
}

if ( fabs( sum - answer ) > 0.01 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}



static void test5(int ndim, int spread, double *params, int *status) {
   int nx = 100;
   int nx_in = 100, ny_in = 200, border_out = 10, nx_out = 120, ny_out = 220;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, k=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sum = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0};
   int in_i[20000]={0};
   double ina[3]={0}, inb[3]={0}, outa[3]={0}, outb[3]={0}, va=0, vb=0;






if ( *status != 0 ) return;
astBegin;

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx_in - 1;
ubnd_in[2-1] = ny_in;

lbnd_out[1-1] = lbnd_in[1-1] - border_out;
lbnd_out[2-1] = lbnd_in[2-1] - border_out;
ubnd_out[1-1] = ubnd_in[1-1] + border_out;
ubnd_out[2-1] = ubnd_in[2-1] + border_out;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__NONORM;

ina[1-1] = lbnd_in[1-1];
ina[2-1] = lbnd_in[2-1];
ina[3-1] = lbnd_in[3-1];
inb[1-1] = ubnd_in[1-1];
inb[2-1] = ubnd_in[2-1];
inb[3-1] = ubnd_in[3-1] + 1.0;

for (k = 1; k <= 3; k++) {

for (i = 1; i <= nx_in; i++) {
for (j = 1; j <= ny_in; j++) {
in_r[(j-1)*nx + i - 1] = k;
in_d[(j-1)*nx + i - 1] = k;
in_i[(j-1)*nx + i - 1] = k;
}
}

va = (k-1)*0.25;
vb = va + 0.5;

outa[1-1] = vb*lbnd_in[1-1] + va*ubnd_in[1-1];
outa[2-1] = vb*lbnd_in[2-1] + va*ubnd_in[2-1];
outb[1-1] = va*lbnd_in[1-1] + vb*ubnd_in[1-1];
outb[2-1] = va*lbnd_in[2-1] + vb*ubnd_in[2-1];
outa[3-1] = ina[3-1];
outb[3-1] = inb[3-1];

map = (AstMapping*)astWinMap( ndim, ina, inb, outa, outb, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)in_d, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)out_d, (double *)weights_d, &nused);

flags = AST__NONORM;
}

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny_out;
}

sum = 0.0;

for (i = 1; i <= nx_out; i++) {
for (j = 1; j <= jhi; j++) {
if ( out_r[(j-1)*nx + i - 1] != AST__BAD ) {
sum = sum + out_r[(j-1)*nx + i - 1];
}
}
}

if ( ndim == 1 ) {
if ( fabs( sum - 600 ) > 1.0E-3 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

}

if ( fabs( out_r[(1-1)*nx + 20 - 1] - 2.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(1-1)*nx + 50 - 1] - 6.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(1-1)*nx + 70 - 1] - 10.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(1-1)*nx + 100 - 1] - 6.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

} else if ( ndim == 2 ) {
if ( fabs( sum - 120000 ) > 1.0E-3 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

}

if ( fabs( out_r[(40-1)*nx + 40 - 1] - 4.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(90-1)*nx + 50 - 1] - 12.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(80-1)*nx + 70 - 1] - 8.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(130-1)*nx + 70 - 1] - 20.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( out_r[(130-1)*nx + 20 - 1] - 0.0 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}




static void test6(int ndim, int spread, double *params, int *status) {
   int nx = 100;
   int nx_in = 100, ny_in = 200, border_out = 10, nx_out = 120, ny_out = 220, nk = 50;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, k=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sum = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0};
   int in_i[20000]={0};
   double ina[3]={0}, inb[3]={0}, outa[3]={0}, outb[3]={0}, va=0, vb=0, answer=0, mxval=0, mnval=0;







if ( *status != 0 ) return;
astBegin;

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx_in - 1;
ubnd_in[2-1] = ny_in;

lbnd_out[1-1] = lbnd_in[1-1] - border_out;
lbnd_out[2-1] = lbnd_in[2-1] - border_out;
ubnd_out[1-1] = ubnd_in[1-1] + border_out;
ubnd_out[2-1] = ubnd_in[2-1] + border_out;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

ina[1-1] = lbnd_in[1-1];
ina[2-1] = lbnd_in[2-1];
ina[3-1] = lbnd_in[3-1];
inb[1-1] = ubnd_in[1-1];
inb[2-1] = ubnd_in[2-1];
inb[3-1] = ubnd_in[3-1] + 1.0;

for (k = 1; k <= nk; k++) {

flags = AST__CONSERVEFLUX;
if ( k == 1 ) flags = flags + AST__REBININIT;
if ( k == nk ) flags = flags + AST__REBINEND;

for (i = 1; i <= nx_in; i++) {
for (j = 1; j <= ny_in; j++) {
in_r[(j-1)*nx + i - 1] = k;
in_d[(j-1)*nx + i - 1] = k;
in_i[(j-1)*nx + i - 1] = k;
}
}

va = (k-1)*0.25;
vb = va + 0.5;

outa[1-1] = vb*lbnd_in[1-1] + va*ubnd_in[1-1];
outa[2-1] = vb*lbnd_in[2-1] + va*ubnd_in[2-1];
outb[1-1] = va*lbnd_in[1-1] + vb*ubnd_in[1-1];
outb[2-1] = va*lbnd_in[2-1] + vb*ubnd_in[2-1];
outa[3-1] = ina[3-1];
outb[3-1] = inb[3-1];

map = (AstMapping*)astWinMap( ndim, ina, inb, outa, outb, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)in_d, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)out_d, (double *)weights_d, &nused);

}

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny_out;
}

sum = 0.0;
mxval = -1e300;
mnval = 1e300;
for (i = 1; i <= nx_out; i++) {
for (j = 1; j <= jhi; j++) {
if ( out_r[(j-1)*nx + i - 1] != AST__BAD ) {
sum = sum + out_r[(j-1)*nx + i - 1];
if ( out_r[(j-1)*nx + i - 1] > mxval ) mxval = out_r[(j-1)*nx + i - 1];
if ( out_r[(j-1)*nx + i - 1] < mnval ) mnval = out_r[(j-1)*nx + i - 1];
}
}
}

if ( ndim == 1 ) {
if ( spread == AST__GAUSS ) {
answer = 414.0;
} else if ( spread == AST__NEAREST ) {
answer = 399.4;
} else if ( spread == AST__LINEAR ) {
answer = 400.0;
} else {
answer = -1.0;
}

if ( fabs( sum - answer ) > 1.0e-3 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( mxval - 6 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( mnval - 2 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

}

} else if ( ndim == 2 ) {
if ( spread == AST__GAUSS ) {
answer = 109011.729592723;
} else if ( spread == AST__NEAREST ) {
answer = 100716.666666667;
} else if ( spread == AST__LINEAR ) {
answer = 102816.0;
} else {
answer = -1.0;
}

if ( fabs( sum - answer ) > 1.0e-3 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( mxval - 12 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( mnval - 4 ) > 1.0E-6 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}


static void test7(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double weights_r[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0};
   int in_i[20000]={0};





if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND;

map = (AstMapping*)astZoomMap( ndim,  0.5, "");
astRebinSeqF(map, 0.0, ndim, lbnd_in, ubnd_in, (const float *)in_r, (const float *)in_r, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (float *)out_r, (float *)out_r, (double *)weights_r, &nused);

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {
if ( fabs( out_r[(j-1)*nx + i - 1] - 1.0 ) > 1.0E-6 && out_r[(j-1)*nx + i - 1] != AST__BAD ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}
}
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}


static void test8(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sum = 0.0;
   double weights_r[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0};
   int in_i[20000]={0};
   double shifts[3]={0};





if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= nx; i++) {
for (j = 1; j <= ny; j++) {
in_r[(j-1)*nx + i - 1] = 1.0;
in_d[(j-1)*nx + i - 1] = 1.0;
in_i[(j-1)*nx + i - 1] = 1.0;
}
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__NONORM;

shifts[1-1] = 5.0;
shifts[2-1] = 5.0;

if ( ndim < 3 ) {
map = (AstMapping*)astCmpMap( (AstMapping*)astZoomMap( ndim,  0.5, ""), (AstMapping*)astShiftMap( ndim, shifts, ""), 1, "" );
} else {
map = (AstMapping*)astCmpMap( (AstMapping*)astCmpMap( (AstMapping*)astZoomMap( 2,  0.5, ""), (AstMapping*)astShiftMap( 2, shifts, ""), 1, "" ), (AstMapping*)astUnitMap( 1, ""), 0, "" );
}

astRebinSeqF(map, 0.0, ndim, lbnd_in, ubnd_in, (const float *)in_r, (const float *)in_r, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (float *)out_r, (float *)out_r, (double *)weights_r, &nused);

if ( ndim == 1 ) {
jhi = 1;
} else {
jhi = ny;
}

sum = 0.0;
for (i = 1; i <= nx; i++) {
for (j = 1; j <= jhi; j++) {
if ( out_r[(j-1)*nx + i - 1] != AST__BAD ) {
sum = sum + (double)(out_r[(j-1)*nx + i - 1]);
}
}
}

if ( sum != sum ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");

} else if ( fabs( sum - nx*jhi ) > sum*1.0e-7 ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}


static void test9(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200, size = 20000;
   int nx1 = 20000, ny1 = 1, nx2 = 100, ny2 = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, jlo=0, ilo=0, ihi=0, k=0, nval=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sigma = 1.0, sum = 0.0, sum2 = 0.0, sum3 = 0.0, realvar = 0.0, meanvar = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0}, vin[20000]={0}, vout[20000]={0};
   int in_i[20000]={0};
   float in_r8[8]={0};







if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= size; i++) {
in_r[ i -1] = 1.0;
in_d[ i -1] = 1.0;
in_i[ i -1] = 1.0;
in_r8[ i -1] = 1.0;
vin[ i -1] = sigma*2;
}

addnoise( size, in_d, sigma, status );

if ( ndim == 1 ) {
nx = nx1;
ny = ny1;
} else {
nx = nx2;
ny = ny2;
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__NONORM + AST__USEVAR;

map = (AstMapping*)astZoomMap( ndim,  0.5, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)vin, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)vout, (double *)weights_d, &nused);

if ( ndim == 1 ) {
ilo = 6;
ihi = (int)round(0.45*nx1);
jlo = 1;
jhi = 1;
} else {
ilo = 6;
ihi = 41;
jlo = 8;
jhi = 91;
}

sum = 0.0;
sum2 = 0.0;
sum3 = 0.0;
nval = 0;

for (i = ilo; i <= ihi; i++) {
for (j = jlo; j <= jhi; j++) {
k = ( j - 1 )*nx + i;
if ( out_r[k-1] != AST__BAD ) {
sum = sum + out_r[k-1];
sum2 = sum2 + (out_d[k-1] * out_r[k-1]);
sum3 = sum3 + vout[k-1];
nval = nval + 1;
}
}
}

sum = sum/nval;
realvar = sum2/nval - sum*sum;
meanvar = sum3/nval;
if ( fabs( realvar - meanvar ) > 0.05*( realvar + meanvar ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}


static void test10(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200, size = 20000;
   int nx1 = 20000, ny1 = 1, nx2 = 100, ny2 = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, jlo=0, ilo=0, ihi=0, k=0, nval=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sigma = 1.0, sum = 0.0, sum2 = 0.0, sum3 = 0.0, realvar = 0.0, meanvar = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0}, vin[20000]={0}, vout[20000]={0};
   int in_i[20000]={0};
   float in_r8[8]={0};







if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= size; i++) {
in_r[ i -1] = 1.0;
in_d[ i -1] = 1.0;
in_i[ i -1] = 1.0;
in_r8[ i -1] = 1.0;
vin[ i -1] = sigma*2;
}

addnoise( size, in_d, sigma, status );

if ( ndim == 1 ) {
nx = nx1;
ny = ny1;
} else {
nx = nx2;
ny = ny2;
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__CONSERVEFLUX + AST__USEVAR;

map = (AstMapping*)astZoomMap( ndim,  0.5, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)vin, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)vout, (double *)weights_d, &nused);

if ( ndim == 1 ) {
ilo = 6;
ihi = (int)round(0.45*nx1);
jlo = 1;
jhi = 1;
} else {
ilo = 6;
ihi = 41;
jlo = 8;
jhi = 91;
}

sum = 0.0;
sum2 = 0.0;
sum3 = 0.0;
nval = 0;

for (i = ilo; i <= ihi; i++) {
for (j = jlo; j <= jhi; j++) {
k = ( j - 1 )*nx + i;
if ( out_r[k-1] != AST__BAD ) {
sum = sum + out_r[k-1];
sum2 = sum2 + (out_d[k-1] * out_r[k-1]);
sum3 = sum3 + vout[k-1];
nval = nval + 1;
}
}
}

sum = sum/nval;
realvar = sum2/nval - sum*sum;
meanvar = sum3/nval;

if ( fabs( realvar - meanvar ) > 0.05*( realvar + meanvar ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}

static void test11(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200, size = 20000;
   int nx1 = 20000, ny1 = 1, nx2 = 100, ny2 = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, jlo=0, ilo=0, ihi=0, k=0, nval=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sigma = 1.0, sum = 0.0, sum2 = 0.0, sum3 = 0.0, realvar = 0.0, meanvar = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0}, vin[20000]={0}, vout[20000]={0};
   int in_i[20000]={0};
   float in_r8[8]={0};







if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= size; i++) {
in_r[ i -1] = 1.0;
in_d[ i -1] = 1.0;
in_i[ i -1] = 1.0;
in_r8[ i -1] = 1.0;
vin[ i -1] = sigma*2;
}

addnoise( size, in_d, sigma, status );

if ( ndim == 1 ) {
nx = nx1;
ny = ny1;
} else {
nx = nx2;
ny = ny2;
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__GENVAR;

map = (AstMapping*)astZoomMap( ndim,  0.5, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)vin, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)vout, (double *)weights_d, &nused);

if ( ndim == 1 ) {
ilo = 6;
ihi = (int)round(0.45*nx1);
jlo = 1;
jhi = 1;
} else {
ilo = 6;
ihi = 41;
jlo = 8;
jhi = 91;
}

sum = 0.0;
sum2 = 0.0;
sum3 = 0.0;
nval = 0;

for (i = ilo; i <= ihi; i++) {
for (j = jlo; j <= jhi; j++) {
k = ( j - 1 )*nx + i;
if ( out_r[k-1] != AST__BAD ) {
sum = sum + out_r[k-1];
sum2 = sum2 + (out_d[k-1] * out_r[k-1]);
sum3 = sum3 + vout[k-1];
nval = nval + 1;
}
}
}

sum = sum/nval;
realvar = sum2/nval - sum*sum;
meanvar = sum3/nval;
if ( fabs( realvar - meanvar ) > 0.05*( realvar + meanvar ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}


static void test12(int ndim, int spread, double *params, int *status) {
   int nx = 100, ny = 200, size = 20000;
   int nx1 = 20000, ny1 = 1, nx2 = 100, ny2 = 200;
   int lbnd_in[3]={0}, ubnd_in[3]={0}, lbnd_out[3]={0}, ubnd_out[3]={0};
   int flags=0, i=0, j=0, jhi=0, jlo=0, ilo=0, ihi=0, k=0, nval=0;
   int64_t nused=0;
   AstMapping *map = NULL;
   double sigma = 1.0, sum = 0.0, sum2 = 0.0, sum3 = 0.0, realvar = 0.0, meanvar = 0.0;
   double weights_d[20000]={0};
   float in_r[20000]={0}, out_r[20000]={0};
   double in_d[20000]={0}, out_d[20000]={0}, vin[20000]={0}, vout[20000]={0};
   int in_i[20000]={0};
   float in_r8[8]={0};







if ( *status != 0 ) return;
astBegin;

for (i = 1; i <= size; i++) {
in_r[ i -1] = 1.0;
in_d[ i -1] = 1.0;
in_i[ i -1] = 1.0;
in_r8[ i -1] = 1.0;
vin[ i -1] = sigma*2;
}

addnoise( size, in_d, sigma, status );

if ( ndim == 1 ) {
nx = nx1;
ny = ny1;
} else {
nx = nx2;
ny = ny2;
}

lbnd_in[1-1] = 0;
lbnd_in[2-1] = 1;
ubnd_in[1-1] = nx - 1;
ubnd_in[2-1] = ny;

lbnd_out[1-1] = 0;
lbnd_out[2-1] = 1;
ubnd_out[1-1] = nx - 1;
ubnd_out[2-1] = ny;

lbnd_in[3-1] = 1;
ubnd_in[3-1] = 1;
lbnd_out[3-1] = 1;
ubnd_out[3-1] = 1;

flags = AST__REBININIT + AST__REBINEND + AST__GENVAR + AST__CONSERVEFLUX + AST__VARWGT;

map = (AstMapping*)astZoomMap( ndim,  0.5, "");
astRebinSeqD(map, 0.0, ndim, lbnd_in, ubnd_in, (const double *)in_d, (const double *)vin, spread, params, flags, 0.01, 50, AST__BAD, ndim, lbnd_out, ubnd_out, lbnd_in, ubnd_in, (double *)out_d, (double *)vout, (double *)weights_d, &nused);

if ( ndim == 1 ) {
ilo = 6;
ihi = (int)round(0.45*nx1);
jlo = 1;
jhi = 1;
} else {
ilo = 6;
ihi = 41;
jlo = 8;
jhi = 91;
}

sum = 0.0;
sum2 = 0.0;
sum3 = 0.0;
nval = 0;

for (i = ilo; i <= ihi; i++) {
for (j = jlo; j <= jhi; j++) {
k = ( j - 1 )*nx + i;
if ( out_r[k-1] != AST__BAD ) {
sum = sum + out_r[k-1];
sum2 = sum2 + (out_d[k-1] * out_r[k-1]);
sum3 = sum3 + vout[k-1];
nval = nval + 1;
}
}
}

sum = sum/nval;
realvar = sum2/nval - sum*sum;
meanvar = sum3/nval;
if ( fabs( realvar - meanvar ) > 0.05*( realvar + meanvar ) ) {
*status = 1;
*status = 1; printf("Error: %s\n", " ");
}

astEnd;
if ( *status != 0 ) {
*status = 1; printf("Error: %s\n", " ");
}

}











