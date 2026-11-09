#include <mpi.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

#include "solutions/Extra.h"
#include "solutions/Sequential.h"
#include "solutions/AsynchronousCommunication.h"
#include "solutions/ScatterGather.h"
#include "solutions/StandardCommunication.h"
#include "utils/number_reader.h"
#include "utils/number_writer.h"

/* The barrier makes all processes start timing at the same moment. */
static double timer_start(void) {
    MPI_Barrier(MPI_COMM_WORLD);
    return MPI_Wtime();
}

/* Returns the slowest process' time (meaningful on process 0 only). */
static double timer_stop(double t0) {
    double local = MPI_Wtime() - t0;
    double max_time = 0.0;
    MPI_Reduce(&local, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    return max_time;
}

static void report(const char *name, double t) {
    std::printf("  %-48s %9.5f s\n", name, t);
}

static void abort_all(int rank, const char* message) {
    if (rank == 0) {
        std::cerr << "Error: " << message << std::endl;
    }
    MPI_Abort(MPI_COMM_WORLD, 1);
}

int main(int argc, char **argv)
{
    // Every process runs the same program; MPI rank determines its role.
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Usage: mpirun -np 5 ./hw4 <variant>, where variant is 0, 1, 1.1, 2, or 3.
    // Default to the sequential variant so it can also be run without arguments.
    int variant = 0;
    bool extra_variant = false;
    if (argc > 1) {
        if (std::strcmp(argv[1], "1.1") == 0) {
            variant = 1;
            extra_variant = true;
        } else {
            char* end = nullptr;
            const long parsed_variant = std::strtol(argv[1], &end, 10);
            if (end == argv[1] || *end != '\0' || parsed_variant < 0 || parsed_variant > 3) {
                if (rank == 0) {
                    std::cerr << "Usage: mpirun -np <processes> ./hw4 <0|1|1.1|2|3>\n";
                }
                MPI_Finalize();
                return 1;
            }
            variant = static_cast<int>(parsed_variant);
        }
        if (variant < 0 || variant > 3) {
            if (rank == 0) {
                std::cerr << "Usage: mpirun -np <processes> ./hw4 <0|1|1.1|2|3>\n";
            }
            MPI_Finalize();
            return 1;
        }
    }

    if (rank == 0)
        std::printf("Processes: %d, variant: %s\n", size,
                    extra_variant ? "1.1" : (variant == 1 ? "1" : variant == 0 ? "0" : variant == 2 ? "2" : "3"));

    double t;
    switch (variant) {
        case 0: {
            /* ---------- Variant 0: sequential implementation ---------- */
            t = timer_start();

            if (rank == 0) {
                try {
                    // File reading is included in the measured interval, as
                    // required by the assignment.
                    InputNumbers numbers = NumberReader::read();
                    const int first_length = static_cast<int>(numbers.first_size);
                    const int second_length = static_cast<int>(numbers.second_size);
                    const int result_capacity = std::max(first_length, second_length) + 1;
                    unsigned char* result = Sequential::add(
                        numbers.first, first_length, numbers.second, second_length);
                    const int result_length =
                        result_capacity - (result[result_capacity - 1] == 0 ? 1 : 0);

                    NumberWriter::write(result, result_length);
                    delete[] result;
                } catch (const std::exception& error) {
                    abort_all(rank, error.what());
                }
            }

            t = timer_stop(t);
            if (rank == 0)
                report("Variant 0 (sequential)", t);
            break;
        }

        case 1: {
            /* ---------- Variant 1 or 1.1: standard Send / Recv ---------- */
            t = timer_start();
            if (extra_variant)
                Extra::add();
            else
                StandardCommunication::add();
            t = timer_stop(t);
            if (rank == 0)
                report(extra_variant ? "Variant 1.1 (precomputed carries)" :
                                       "Variant 1 (standard communication)", t);
            break;
        }

        case 2: {
            /* ---------- Variant 2: Scatter / Gather ---------- */
            t = timer_start();
            ScatterGather::add();
            t = timer_stop(t);
            if (rank == 0)
                report("Variant 2 (Scatter / Gather)", t);
            break;
        }

        case 3: {
            /* ---------- Variant 3: asynchronous communication ---------- */
            t = timer_start();
            AsynchronousCommunication::add();
            t = timer_stop(t);
            if (rank == 0)
                report("Variant 3 (asynchronous communication)", t);
            break;
        }
    }

    // MPI_Finalize shuts down MPI after all processes complete their work.
    MPI_Finalize();
    return 0;
}
