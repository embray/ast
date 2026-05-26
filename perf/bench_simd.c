/* Enable POSIX 2008 (clock_gettime) and glibc extensions (drand48). */
#define _DEFAULT_SOURCE

/*
 * bench_simd.c
 *
 * Benchmark AST transforms at varying N to evaluate SIMD vectorization gains.
 *
 * Benchmarks:
 *   sphmap_fwd  — SphMap (x,y,z) -> (lon,lat): inlined atan2/sqrt via libmvec
 *   sphmap_inv  — SphMap (lon,lat) -> (x,y,z): inlined sin/cos via libmvec
 *   poly5_fwd   — degree-5 2-D PolyMap (Roman WCS SIP-like distortion)
 *   poly1_fwd   — degree-1 2-D PolyMap (Roman WCS rotation/scale)
 *
 * When built with AST_ENABLE_SIMD the trig calls inside the SphMap transform
 * loop are dispatched to libmvec vector variants (_ZGVdN4vv_atan2,
 * _ZGVdN4v_sin, _ZGVdN4v_cos) via #pragma omp simd.  PolyMap results
 * establish a baseline for the planned Phase 4 SIMD work.
 *
 * Output CSV:
 *   transform,n_points,rep,time_s
 *
 * Usage:
 *   bench_simd [-o output.csv] [-r nreps]
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ast.h"

/* N sweep: 1 through 4088*4088 (full Roman WFI science area). */
static const size_t N_SWEEP[] = {
    1, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 16384, 65536,
    262144, 1048576, 4194304, 16711744
};
static const size_t N_N_SWEEP = sizeof(N_SWEEP) / sizeof(N_SWEEP[0]);

#define RAND_SEED 42UL
#define DEFAULT_REPS 5
#define IMAGE_HALF 2048.0   /* pixel coordinate half-range for poly inputs */

/*
 * Degree-5 2-D polynomial coefficients from a representative Roman WCS
 * (two independent Polynomial2D models combined into one 2-in/2-out PolyMap).
 * Only non-zero terms are listed.
 * Row format: { coeff, out_coord (1-based), pow_x, pow_y }.
 */
#define POLY5_NCOEFF 10
static const double POLY5_COEFF_F[POLY5_NCOEFF * 4] = {
     0.11034133, 1, 1, 0,
    -3.0e-8,     1, 2, 0,
     3.4168e-4,  1, 0, 1,
    -1.0e-8,     1, 0, 2,
     1.4e-7,     1, 1, 1,
     3.1436e-4,  2, 1, 0,
     7.0e-8,     2, 2, 0,
     0.10828278, 2, 0, 1,
     2.1e-7,     2, 0, 2,
    -2.0e-8,     2, 1, 1,
};


/*
 * Degree-1 2-D polynomial coefficients from the same Roman WCS (a ~30deg CCW
 * rotation: [8] gives x_out, [9] gives y_out in the Python model numbering).
 */
#define POLY1_NCOEFF 4
static const double POLY1_COEFF_F[POLY1_NCOEFF * 4] = {
    -0.5,       1, 1, 0,
    -0.8660254, 1, 0, 1,
    -0.8660254, 2, 1, 0,
     0.5,       2, 0, 1,
};


static double now_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}


/*
 * Run the N-sweep for one transform direction and write CSV rows to fout.
 * Returns 0 on success, 1 if astTranP fails.
 */
static int run_sweep(FILE *fout, AstMapping *map, const char *direction,
                     int forward, int nin, const double **ptr_in,
                     int nout, double **ptr_out,
                     int nreps, double *rep_times,
                     const char *label) {
    fprintf(stderr, "=== %s%s ===\n", direction, label);

    for (size_t ni = 0; ni < N_N_SWEEP; ni++) {
        size_t N = N_SWEEP[ni];

        for (int rep = 0; rep < nreps; rep++) {
            double t0 = now_s();
            astTranP(map, (int)N, nin, ptr_in, forward, nout, ptr_out);
            rep_times[rep] = now_s() - t0;

            if (!astOK) {
                fprintf(stderr, "error: astTranP failed\n");
                return 1;
            }

            fprintf(fout, "%s,%zu,%d,%.9f\n", direction, N, rep, rep_times[rep]);
        }
        fflush(fout);

        for (int a = 0; a < nreps - 1; a++)
            for (int b = a + 1; b < nreps; b++)
                if (rep_times[b] < rep_times[a]) {
                    double tmp = rep_times[a];
                    rep_times[a] = rep_times[b];
                    rep_times[b] = tmp;
                }
        double median = rep_times[nreps / 2];
        fprintf(stderr, "  %s  N=%-8zu  median=%8.3f ms  (%6.1f Mpx/s)\n",
                direction, N, median * 1e3, (double)N / median / 1e6);
    }
    return 0;
}


/*
 * Run forward and inverse SphMap sweeps.  x/y/z are Cartesian inputs,
 * lon/lat are spherical inputs; out0/out1/out2 are shared output buffers.
 */
static int bench_sphmap(FILE *fout, AstSphMap *sphmap,
                        const double *x, const double *y, const double *z,
                        const double *lon, const double *lat,
                        double *out0, double *out1, double *out2,
                        int nreps, double *rep_times, const char *simd_label) {
    const double *fwd_in[3] = { x, y, z };
    double *fwd_out[2] = { out0, out1 };
    const double *inv_in[2] = { lon, lat };
    double *inv_out[3] = { out0, out1, out2 };
    int rc = 0;

    rc = run_sweep(fout, (AstMapping *)sphmap, "sphmap_fwd", 1,
                   3, fwd_in, 2, fwd_out, nreps, rep_times, simd_label);

    if (rc)
        return rc;

    return run_sweep(fout, (AstMapping *)sphmap, "sphmap_inv", 0,
                     2, inv_in, 3, inv_out, nreps, rep_times, simd_label);
}


/*
 * Create a forward-only 2-in/2-out PolyMap from coeff_f, run the N-sweep,
 * and return 0 on success.  in0/in1 must be pre-filled by the caller.
 */
static int bench_polymap(FILE *fout, const char *label,
                         int ncoeff, const double *coeff_f,
                         const double *in0, const double *in1,
                         double *out0, double *out1,
                         int nreps, double *rep_times,
                         const char *simd_label) {
    AstPolyMap *polymap = astPolyMap(2, 2, ncoeff, coeff_f, 0, NULL, " ");
    if (!astOK) {
        fprintf(stderr, "error: failed to create PolyMap '%s'\n", label);
        return 1;
    }

    const double *ptr_in[2] = { in0, in1 };
    double *ptr_out[2] = { out0, out1 };

    return run_sweep(fout, (AstMapping *)polymap, label, 1,
                     2, ptr_in, 2, ptr_out, nreps, rep_times, simd_label);
}


int main(int argc, char *argv[]) {
    const char *outpath = NULL;
    int nreps = DEFAULT_REPS;
    FILE *fout = stdout;
    int status = 0;
    int rc = 0;
    size_t max_n = N_SWEEP[N_N_SWEEP - 1];
    double *x = NULL;
    double *y = NULL;
    double *z = NULL;
    double *lon = NULL;
    double *lat = NULL;
    double *out0 = NULL;
    double *out1 = NULL;
    double *out2 = NULL;
    double *rep_times = NULL;
    AstSphMap *sphmap = NULL;

    const char *simd_label =
#ifdef AST_HAVE_SIMD
        " [SIMD]";
#else
        "";
#endif

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outpath = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            nreps = atoi(argv[++i]);
            if (nreps < 1) nreps = 1;
        } else {
            fprintf(stderr, "Usage: %s [-o output.csv] [-r nreps]\n", argv[0]);
            return 1;
        }
    }

    if (outpath) {
        fout = fopen(outpath, "w");
        if (!fout) {
            fprintf(stderr, "error: cannot open %s for writing\n", outpath);
            return 1;
        }
    }

    astWatch(&status);
    astBegin;

    sphmap = astSphMap("UnitRadius=1");
    if (!astOK) {
        fprintf(stderr, "error: failed to create SphMap (status=%d)\n", status);
        rc = 1;
        goto done;
    }

    /* Forward input: 3 Cartesian coords. */
    x = malloc(max_n * sizeof(double));
    y = malloc(max_n * sizeof(double));
    z = malloc(max_n * sizeof(double));
    /* Inverse input: 2 spherical coords. */
    lon = malloc(max_n * sizeof(double));
    lat = malloc(max_n * sizeof(double));
    /* Outputs (3 needed for inverse). */
    out0 = malloc(max_n * sizeof(double));
    out1 = malloc(max_n * sizeof(double));
    out2 = malloc(max_n * sizeof(double));
    rep_times = malloc((size_t)nreps * sizeof(double));

    if (!x || !y || !z || !lon || !lat || !out0 || !out1 || !out2 || !rep_times) {
        fprintf(stderr, "error: out of memory\n");
        rc = 1;
        goto done;
    }

    /* Random unit-sphere Cartesian points and matching spherical coords. */
    srand48((long)RAND_SEED);
    for (size_t i = 0; i < max_n; i++) {
        double lo = drand48() * 2.0 * M_PI;
        double la = (drand48() - 0.5) * M_PI;
        double cp = cos(la);
        x[i] = cos(lo) * cp;
        y[i] = sin(lo) * cp;
        z[i] = sin(la);
        lon[i] = lo;
        lat[i] = la;
    }

    fprintf(fout, "transform,n_points,rep,time_s\n");

    rc = bench_sphmap(fout, sphmap, x, y, z, lon, lat, out0, out1, out2,
                      nreps, rep_times, simd_label);
    if (rc) goto done;

    /* Reinitialise x/y as centred pixel coordinates for polynomial benchmarks. */
    srand48((long)RAND_SEED);
    for (size_t i = 0; i < max_n; i++) {
        x[i] = (drand48() - 0.5) * 2.0 * IMAGE_HALF;
        y[i] = (drand48() - 0.5) * 2.0 * IMAGE_HALF;
    }

    rc = bench_polymap(fout, "poly5_fwd", POLY5_NCOEFF, POLY5_COEFF_F,
                       x, y, out0, out1, nreps, rep_times, simd_label);
    if (rc) goto done;

    rc = bench_polymap(fout, "poly1_fwd", POLY1_NCOEFF, POLY1_COEFF_F,
                       x, y, out0, out1, nreps, rep_times, simd_label);

done:
    free(rep_times);
    free(x);
    free(y);
    free(z);
    free(lon);
    free(lat);
    free(out0);
    free(out1);
    free(out2);

    astEnd;

    if (outpath)
        fclose(fout);

    return (rc || status) ? 1 : 0;
}
