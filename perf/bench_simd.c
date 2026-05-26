/* Enable POSIX 2008 (clock_gettime) and glibc extensions (drand48). */
#define _DEFAULT_SOURCE

/*
 * bench_simd.c
 *
 * Benchmark AST transforms at varying N to evaluate SIMD vectorization gains.
 *
 * Currently benchmarks SphMap forward and inverse transforms:
 *   Forward: (x,y,z) -> (lon,lat): inlined atan2/sqrt via libmvec
 *   Inverse: (lon,lat) -> (x,y,z): inlined sin/cos via libmvec
 *
 * When built with AST_ENABLE_SIMD the trig calls inside the transform loop
 * are dispatched to libmvec vector variants (_ZGVdN4vv_atan2, _ZGVdN4v_sin,
 * _ZGVdN4v_cos) via #pragma omp simd.
 *
 * Output CSV:
 *   direction,n_points,rep,time_s
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
#define DEFAULT_REPS  5


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
                     const char *label)
{
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

    fprintf(fout, "direction,n_points,rep,time_s\n");

    {
        const double *fwd_in[3] = { x, y, z };
        double       *fwd_out[2] = { out0, out1 };
        const double *inv_in[2] = { lon, lat };
        double       *inv_out[3] = { out0, out1, out2 };

        /* Forward: (x,y,z) -> (lon,lat) */
        rc = run_sweep(fout, (AstMapping *)sphmap, "forward", 1,
                       3, fwd_in, 2, fwd_out, nreps, rep_times, simd_label);
        if (rc) goto done;

        /* Inverse: (lon,lat) -> (x,y,z) */
        rc = run_sweep(fout, (AstMapping *)sphmap, "inverse", 0,
                       2, inv_in, 3, inv_out, nreps, rep_times, simd_label);
    }

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
