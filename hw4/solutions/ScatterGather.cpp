#include "ScatterGather.h"

#include <mpi.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "../utils/number_reader.h"
#include "../utils/number_writer.h"

using namespace std;

void ScatterGather::add() {
    const int TAG_CARRY = 1;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    unsigned char* first = nullptr;
    unsigned char* second = nullptr;
    unsigned char* result = nullptr;
    unsigned char* local_first = nullptr;
    unsigned char* local_second = nullptr;
    unsigned char* local_result = nullptr;

    try {
        int first_length = 0;
        int second_length = 0;
        int total_length = 0;
        int chunk = 0;
        int padded_length = 0;

        if (rank == 0) {
            // Process 0 reads both numbers. Other processes receive only their
            // padded digit blocks through MPI_Scatter.
            InputNumbers numbers = NumberReader::read();
            if (numbers.first_size > static_cast<size_t>(numeric_limits<int>::max()) ||
                numbers.second_size > static_cast<size_t>(numeric_limits<int>::max())) {
                throw runtime_error("Input number is too long.");
            }

            first_length = static_cast<int>(numbers.first_size);
            second_length = static_cast<int>(numbers.second_size);
            total_length = max(first_length, second_length);

            // MPI_Scatter requires equal-sized blocks, so pad to a multiple of
            // the process count. The shorter input is padded with zero digits.
            chunk = total_length / size + (total_length % size != 0);
            padded_length = chunk * size;

            first = new unsigned char[padded_length]();
            second = new unsigned char[padded_length]();
            for (int i = 0; i < first_length; ++i) {
                first[i] = numbers.first[i];
            }
            for (int i = 0; i < second_length; ++i) {
                second[i] = numbers.second[i];
            }
        }

        int metadata[4] = {first_length, second_length, total_length, chunk};
        // MPI_Bcast sends process 0's lengths and common chunk size to all ranks.
        MPI_Bcast(metadata, 4, MPI_INT, 0, MPI_COMM_WORLD);
        first_length = metadata[0];
        second_length = metadata[1];
        total_length = metadata[2];
        chunk = metadata[3];
        padded_length = chunk * size;

        local_first = new unsigned char[chunk];
        local_second = new unsigned char[chunk];
        local_result = new unsigned char[chunk];

        // MPI_Scatter distributes one equal-sized digit block to every process.
        // Process 0 participates and receives its own first block as well.
        MPI_Scatter(first, chunk, MPI_UNSIGNED_CHAR, local_first, chunk,
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
        MPI_Scatter(second, chunk, MPI_UNSIGNED_CHAR, local_second, chunk,
                    MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Add this block assuming no incoming carry, and save its carry-out.
        int local_carry = 0;
        const int first_digit_index = rank * chunk;
        int valid_digits = total_length - first_digit_index;
        if (valid_digits > chunk) {
            valid_digits = chunk;
        }
        for (int i = 0; i < chunk; ++i) {
            if (i < valid_digits) {
                const int sum = local_first[i] + local_second[i] + local_carry;
                local_result[i] = static_cast<unsigned char>(sum % 10);
                local_carry = sum / 10;
            } else {
                // Padding makes the Scatter blocks equal-sized, but padded
                // zeroes are not extra digits of the input numbers.
                local_result[i] = 0;
            }
        }

        int incoming_carry = 0;
        if (rank > 0) {
            // Each process receives the carry from its lower-rank neighbor.
            MPI_Recv(&incoming_carry, 1, MPI_INT, rank - 1, TAG_CARRY,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // Apply the incoming carry to the least-significant end of this block.
        int carry_to_propagate = incoming_carry;
        for (int i = 0; i < valid_digits && carry_to_propagate != 0; ++i) {
            if (local_result[i] == 9) {
                local_result[i] = 0;
            } else {
                ++local_result[i];
                carry_to_propagate = 0;
            }
        }

        const int outgoing_carry = local_carry || carry_to_propagate;
        if (rank < size - 1) {
            // Forward the carry to the next process; the last rank has no next
            // process, so it does not send one.
            MPI_Send(&outgoing_carry, 1, MPI_INT, rank + 1, TAG_CARRY,
                     MPI_COMM_WORLD);
        }

        if (rank == 0) {
            result = new unsigned char[padded_length + 1]();
        }

        // MPI_Gather collects all equal-sized result blocks in rank order.
        MPI_Gather(local_result, chunk, MPI_UNSIGNED_CHAR, result, chunk,
                   MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        int final_carry = 0;
        if (size == 1) {
            final_carry = outgoing_carry;
        } else if (rank == size - 1) {
            // Process 0 needs the final carry as well as the gathered digits.
            MPI_Send(&outgoing_carry, 1, MPI_INT, 0, TAG_CARRY, MPI_COMM_WORLD);
        }

        if (rank == 0) {
            if (size > 1) {
                MPI_Recv(&final_carry, 1, MPI_INT, size - 1, TAG_CARRY,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            result[total_length] = static_cast<unsigned char>(final_carry);
            const int result_length = total_length + final_carry;
            NumberWriter::write(result, result_length);
        }

        delete[] first;
        delete[] second;
        delete[] result;
        delete[] local_first;
        delete[] local_second;
        delete[] local_result;
    } catch (const exception& error) {
        if (rank == 0) {
            cerr << "Error: " << error.what() << endl;
        }

        delete[] first;
        delete[] second;
        delete[] result;
        delete[] local_first;
        delete[] local_second;
        delete[] local_result;

        // Stop all ranks if one cannot continue, rather than leave others
        // waiting in an MPI call.
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}
