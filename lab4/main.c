/*
* MPI
*
* Build:  mpicc -O2 main_mpi_en.c -o main_mpi_en
* Run:    mpirun -np 4 ./main_mpi_en [N]     (at least 2 processes; N defaults to 1000003)
*         add --oversubscribe if you ask for more processes than cores
*
* Element-wise product c[i] = a[i] * b[i]
*    A. process 0 sends segments with MPI_Send / MPI_Recv and rebuilds the result
*    B. MPI_Scatter + MPI_Gather
* Dot product sum(a[i] * b[i])
*    Variant 1: MPI_Scatter, then local sums are sent to process 0
*        1.1 received with MPI_Recv  (blocking)
*        1.2 received with MPI_Irecv (non-blocking) + MPI_Waitall
*    Variant 2: MPI_Scatter + MPI_Reduce
*
* Every process runs the same program; its rank decides what it does.
* Processes do not share memory: data moves only through messages.
*
* Timing: MPI_Wtime(), taking the MAX over all processes (an operation is only
* finished when the slowest process is). Each result is checked against the
* sequential one. With so little computation per element, communication
* dominates and the parallel versions may be slower than the sequential one.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define DEFAULT_N 1000003   /* deliberately not divisible by the number of processes */

/* Message tags: let the receiver tell the different messages apart. */
#define TAG_A       1
#define TAG_B       2
#define TAG_RESULT  3
#define TAG_PARTIAL 4

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Splits n elements into "parts" nearly equal pieces: piece idx gets *count
* elements starting at *offset. The first (n % parts) pieces get one extra element.
* Example: n = 10, parts = 3  ->  counts 4,3,3  offsets 0,4,7 */
static void split(int n, int parts, int idx, int *count, int *offset)
{
    int base = n / parts;
    int rem  = n % parts;
    *count  = base + (idx < rem ? 1 : 0);
    *offset = idx * base + (idx < rem ? idx : rem);
}

/* Dot product of two local arrays. */
static double dot_local(const double *x, const double *y, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++)
        s += x[i] * y[i];
    return s;
}

/* The barrier makes all processes start measuring at the same moment. */
static double timer_start(void)
{
    MPI_Barrier(MPI_COMM_WORLD);
    return MPI_Wtime();
}

/* Returns the time of the slowest process (meaningful on process 0 only). */
static double timer_stop(double t0)
{
    double local = MPI_Wtime() - t0;
    double max_time = 0.0;
    MPI_Reduce(&local, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    return max_time;
}

static void report(const char *name, double t, double t_seq, int ok)
{
    printf("  %-48s %9.5f s   speedup x%.2f  %s\n", name, t, t_seq / t, ok ? "" : "  !!! WRONG RESULT");
}

/* ------------------------------------------------------------------ */
/* A.   Element-wise product with Send / Recv                          */
/* ------------------------------------------------------------------ */
/* Process 0 only distributes and collects; the workers are processes 1..size-1.
* a, b and res are used on process 0 only (NULL elsewhere). */
static void vec_mul_send_recv(int rank, int size, int n,
                              const double *a, const double *b, double *res)
{
    int workers = size - 1;
    int count, offset;

    if (rank == 0) {
        /* Send worker w its segment (piece w-1 of the split). */
        for (int w = 1; w <= workers; w++) {
            split(n, workers, w - 1, &count, &offset);
            MPI_Send(a + offset, count, MPI_DOUBLE, w, TAG_A, MPI_COMM_WORLD);
            MPI_Send(b + offset, count, MPI_DOUBLE, w, TAG_B, MPI_COMM_WORLD);
        }
        /* Rebuild the result: each piece goes back to its own offset. */
        for (int w = 1; w <= workers; w++) {
            split(n, workers, w - 1, &count, &offset);
            MPI_Recv(res + offset, count, MPI_DOUBLE, w, TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        /* A worker computes its own segment size with the same formula as the master. */
        split(n, workers, rank - 1, &count, &offset);
        int cap = count > 0 ? count : 1;
        double *la = malloc(cap * sizeof(double));
        double *lb = malloc(cap * sizeof(double));
        double *lr = malloc(cap * sizeof(double));

        MPI_Recv(la, count, MPI_DOUBLE, 0, TAG_A, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(lb, count, MPI_DOUBLE, 0, TAG_B, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < count; i++)
            lr[i] = la[i] * lb[i];

        MPI_Send(lr, count, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);

        free(la); free(lb); free(lr);
    }
}

/* ------------------------------------------------------------------ */
/* B.   Element-wise product with Scatter / Gather                     */
/* ------------------------------------------------------------------ */
/* MPI_Scatter sends pieces of EQUAL size, so the vectors are zero-padded to
* n_pad = chunk * size in main(). (MPI_Scatterv/Gatherv would allow uneven pieces.)
* Unlike A, process 0 also takes a piece and computes. */
static void vec_mul_scatter_gather(int chunk, const double *a, const double *b, double *res)
{
    double *la = malloc(chunk * sizeof(double));
    double *lb = malloc(chunk * sizeof(double));
    double *lr = malloc(chunk * sizeof(double));

    /* Piece k of a (and b) goes to the process with rank k. */
    MPI_Scatter(a, chunk, MPI_DOUBLE, la, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, chunk, MPI_DOUBLE, lb, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < chunk; i++)
        lr[i] = la[i] * lb[i];

    /* Gather is the inverse: the pieces are concatenated in rank order into res. */
    MPI_Gather(lr, chunk, MPI_DOUBLE, res, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    free(la); free(lb); free(lr);
}

/* ------------------------------------------------------------------ */
/* 1.1  Dot product: Scatter + blocking MPI_Recv                       */
/* ------------------------------------------------------------------ */
/* The returned value is valid on process 0 only. */
static double dot_scatter_recv(int rank, int size, int chunk, const double *a, const double *b)
{
    double *la = malloc(chunk * sizeof(double));
    double *lb = malloc(chunk * sizeof(double));
    MPI_Scatter(a, chunk, MPI_DOUBLE, la, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, chunk, MPI_DOUBLE, lb, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local = dot_local(la, lb, chunk);
    double total = 0.0;

    if (rank == 0) {
        total = local;
        for (int src = 1; src < size; src++) {
            double partial;
            /* Blocks until the sum from process "src" arrives; sums are taken in rank order. */
            MPI_Recv(&partial, 1, MPI_DOUBLE, src, TAG_PARTIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total += partial;
        }
    } else {
        MPI_Send(&local, 1, MPI_DOUBLE, 0, TAG_PARTIAL, MPI_COMM_WORLD);
    }

    free(la); free(lb);
    return total;
}

/* ------------------------------------------------------------------ */
/* 1.2  Dot product: Scatter + non-blocking MPI_Irecv                  */
/* ------------------------------------------------------------------ */
static double dot_scatter_irecv(int rank, int size, int chunk, const double *a, const double *b)
{
    double *la = malloc(chunk * sizeof(double));
    double *lb = malloc(chunk * sizeof(double));
    MPI_Scatter(a, chunk, MPI_DOUBLE, la, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, chunk, MPI_DOUBLE, lb, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double total = 0.0;

    if (rank == 0) {
        double *partial = malloc(size * sizeof(double));            /* partial[r] = sum of process r */
        MPI_Request *req = malloc((size - 1) * sizeof(MPI_Request));

        /* Irecv returns immediately; partial[src] must not be read before the wait below. */
        for (int src = 1; src < size; src++)
            MPI_Irecv(&partial[src], 1, MPI_DOUBLE, src, TAG_PARTIAL, MPI_COMM_WORLD, &req[src - 1]);

        /* Useful work while the messages are in flight: process 0 computes its own sum. */
        partial[0] = dot_local(la, lb, chunk);

        MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);

        for (int i = 0; i < size; i++)
            total += partial[i];

        free(partial); free(req);
    } else {
        double local = dot_local(la, lb, chunk);
        MPI_Send(&local, 1, MPI_DOUBLE, 0, TAG_PARTIAL, MPI_COMM_WORLD);
    }

    free(la); free(lb);
    return total;
}

/* ------------------------------------------------------------------ */
/* 2.   Dot product: Scatter + MPI_Reduce                              */
/* ------------------------------------------------------------------ */
/* MPI_Reduce combines one value per process with MPI_SUM and delivers the result
* to process 0. This replaces the manual Send/Recv loop of variant 1 with one call. */
static double dot_scatter_reduce(int chunk, const double *a, const double *b)
{
    double *la = malloc(chunk * sizeof(double));
    double *lb = malloc(chunk * sizeof(double));
    MPI_Scatter(a, chunk, MPI_DOUBLE, la, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(b, chunk, MPI_DOUBLE, lb, chunk, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double local = dot_local(la, lb, chunk);
    double total = 0.0;
    MPI_Reduce(&local, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    free(la); free(lb);
    return total;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);        /* this process: 0 .. size-1 */
    MPI_Comm_size(MPI_COMM_WORLD, &size);        /* number of processes */

    if (size < 2) {                              /* A needs a master and at least one worker */
        if (rank == 0)
            fprintf(stderr, "Run with at least 2 processes: mpirun -np 4 ./main_mpi_en\n");
        MPI_Finalize();
        return 1;
    }

    int n = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;
    if (n < 1) {
        if (rank == 0) fprintf(stderr, "Invalid N\n");
        MPI_Finalize();
        return 1;
    }

    /* Piece size and padded length required by MPI_Scatter / MPI_Gather. */
    int chunk = (n + size - 1) / size;           /* ceil(n / size) */
    int n_pad = chunk * size;

    /* Only process 0 holds the full vectors. */
    double *a = NULL, *b = NULL, *res = NULL, *ref = NULL;

    if (rank == 0) {
        printf("Processes: %d, N = %d (Scatter/Gather chunk = %d, padded to %d)\n",
               size, n, chunk, n_pad);

        a   = calloc(n_pad, sizeof(double));     /* padding stays 0, so it does not change the dot product */
        b   = calloc(n_pad, sizeof(double));
        res = calloc(n_pad, sizeof(double));
        ref = calloc(n_pad, sizeof(double));

        /* Multiples of 0.5: all sums and products are exact in double, so parallel
         * results can be compared with == to the sequential ones. */
        for (int i = 0; i < n; i++) {
            a[i] = (i % 100) * 0.5;
            b[i] = (i % 50) + 1;
        }
    }

    double t, t_seq = 1.0, dot_ref = 0.0;
    int ok;

    /* ---------- Sequential reference (process 0 only) ---------- */
    if (rank == 0) {
        printf("\n=== Element-wise vector product ===\n");
        t = MPI_Wtime();
        for (int i = 0; i < n; i++)
            ref[i] = a[i] * b[i];
        t_seq = MPI_Wtime() - t;
        report("sequential", t_seq, t_seq, 1);
    }

    /* ---------- A. Send / Recv ---------- */
    t = timer_start();
    vec_mul_send_recv(rank, size, n, a, b, res);
    t = timer_stop(t);
    if (rank == 0) {
        ok = memcmp(res, ref, n * sizeof(double)) == 0;
        report("A. process 0 distributes (Send/Recv)", t, t_seq, ok);
    }

    /* ---------- B. Scatter / Gather ---------- */
    if (rank == 0) memset(res, 0, n_pad * sizeof(double));   /* so the check cannot pass on A's result */
    t = timer_start();
    vec_mul_scatter_gather(chunk, a, b, res);
    t = timer_stop(t);
    if (rank == 0) {
        ok = memcmp(res, ref, n * sizeof(double)) == 0;
        report("B. Scatter + Gather", t, t_seq, ok);
    }

    /* ---------- Dot product: sequential reference ---------- */
    if (rank == 0) {
        printf("\n=== Dot product ===\n");
        t = MPI_Wtime();
        dot_ref = dot_local(a, b, n);
        t_seq = MPI_Wtime() - t;
        report("sequential", t_seq, t_seq, 1);
    }

    double dot;

    /* ---------- 1.1 Scatter + Recv ---------- */
    t = timer_start();
    dot = dot_scatter_recv(rank, size, chunk, a, b);
    t = timer_stop(t);
    if (rank == 0)
        report("1.1 Scatter + local sums with MPI_Recv", t, t_seq, dot == dot_ref);

    /* ---------- 1.2 Scatter + Irecv ---------- */
    t = timer_start();
    dot = dot_scatter_irecv(rank, size, chunk, a, b);
    t = timer_stop(t);
    if (rank == 0)
        report("1.2 Scatter + local sums with MPI_Irecv", t, t_seq, dot == dot_ref);

    /* ---------- 2. Scatter + Reduce ---------- */
    t = timer_start();
    dot = dot_scatter_reduce(chunk, a, b);
    t = timer_stop(t);
    if (rank == 0)
        report("2. Scatter + MPI_Reduce", t, t_seq, dot == dot_ref);

    if (rank == 0) {
        printf("\nDot product = %.1f\n", dot_ref);
        free(a); free(b); free(res); free(ref);
    }

    MPI_Finalize();
    return 0;
}