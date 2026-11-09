#include "StandardCommunication.h"

#include <mpi.h>

#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace std;

#ifdef HW4_USE_SYNCHRONOUS_SEND
#define SEND_MESSAGE MPI_Ssend
#else
#define SEND_MESSAGE MPI_Send
#endif

void StandardCommunication::add() {
    add(false);
}

void StandardCommunication::add(bool optimized) {
    const int TAG_LENGTHS = 1;
    const int TAG_FIRST_DIGITS = 2;
    const int TAG_SECOND_DIGITS = 3;
    const int TAG_CARRY = 4;
    const int TAG_RESULT = 5;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            cerr << "Variant 1 requires at least two processes.\n";
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    try {
        if (rank == 0) {
            const char* first_file = "../number1.txt";
            const char* second_file = "../number2.txt";
            const char* result_file = "../number3.txt";

            ifstream first_input(first_file);
            ifstream second_input(second_file);
            if (!first_input || !second_input) {
                throw runtime_error("Unable to open number1.txt or number2.txt.");
            }

            long long first_length_long, second_length_long;
            if (!(first_input >> first_length_long) ||
                !(second_input >> second_length_long) ||
                first_length_long <= 0 || second_length_long <= 0) {
                throw runtime_error("Invalid digit count in input file.");
            }

            const int first_length = static_cast<int>(first_length_long);
            const int second_length = static_cast<int>(second_length_long);
            const int total_length = first_length > second_length
                                         ? first_length
                                         : second_length;
            const int workers = size - 1;

            ofstream output(result_file);
            if (!output) {
                throw runtime_error("Unable to open number3.txt.");
            }

            // Reserve one digit for a possible carry. The final length is
            // written into the header after all workers return their results.
            output << total_length + 1 << '\n';

            int lengths[3] = {first_length, second_length, total_length};
            for (int worker = 1; worker < size; ++worker) {
                // MPI_Send transfers metadata to the matching worker receive.
                // Use MPI_Ssend instead when testing with synchronous sends.
                SEND_MESSAGE(lengths, 3, MPI_INT, worker, TAG_LENGTHS,
                             MPI_COMM_WORLD);
            }

            int first_read = 0;
            int second_read = 0;

            // Give the workers nearly equal, contiguous digit ranges. Any
            // remainder digits go to the workers with the lowest ranks.
            for (int worker = 1; worker < size; ++worker) {
                const int index = worker - 1;
                const int base = total_length / workers;
                const int remainder = total_length % workers;
                const int count = base + (index < remainder ? 1 : 0);
                const int offset = index * base +
                                   (index < remainder ? index : remainder);

                unsigned char* first_digits = new unsigned char[count > 0 ? count : 1];
                unsigned char* second_digits = new unsigned char[count > 0 ? count : 1];

                // Read only this worker's digits now, not the entire number
                // before distribution. Pad the shorter number with zeros.
                for (int i = 0; i < count; ++i) {
                    const int digit_index = offset + i;
                    if (digit_index < first_length) {
                        int digit;
                        if (!(first_input >> digit) || digit < 0 || digit > 9) {
                            throw runtime_error("Invalid or missing digit in number1.txt.");
                        }
                        first_digits[i] = static_cast<unsigned char>(digit);
                        ++first_read;
                    } else {
                        first_digits[i] = 0;
                    }

                    if (digit_index < second_length) {
                        int digit;
                        if (!(second_input >> digit) || digit < 0 || digit > 9) {
                            throw runtime_error("Invalid or missing digit in number2.txt.");
                        }
                        second_digits[i] = static_cast<unsigned char>(digit);
                        ++second_read;
                    } else {
                        second_digits[i] = 0;
                    }
                }

                // Send both digit blocks before the worker waits for its carry.
                SEND_MESSAGE(first_digits, count, MPI_UNSIGNED_CHAR, worker,
                             TAG_FIRST_DIGITS, MPI_COMM_WORLD);
                SEND_MESSAGE(second_digits, count, MPI_UNSIGNED_CHAR, worker,
                             TAG_SECOND_DIGITS, MPI_COMM_WORLD);

                delete[] first_digits;
                delete[] second_digits;
            }

            string extra;
            if (first_read != first_length || second_read != second_length ||
                (first_input >> extra) || (second_input >> extra)) {
                throw runtime_error("Input digit count does not match its header.");
            }

            bool first_result_digit = true;
            int final_carry = 0;

            // Receive result pieces in rank order and write each piece as it
            // arrives. This preserves least-significant-first file order.
            for (int worker = 1; worker < size; ++worker) {
                const int index = worker - 1;
                const int base = total_length / workers;
                const int remainder = total_length % workers;
                const int count = base + (index < remainder ? 1 : 0);
                unsigned char* result = new unsigned char[count + 1];

                // MPI_Recv blocks until this worker's result and carry arrive.
                MPI_Recv(result, count + 1, MPI_UNSIGNED_CHAR, worker,
                         TAG_RESULT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 0; i < count; ++i) {
                    if (!first_result_digit) {
                        output << ' ';
                    }
                    output << static_cast<unsigned int>(result[i]);
                    first_result_digit = false;
                }

                if (worker == size - 1) {
                    final_carry = result[count];
                }

                delete[] result;
            }

            if (final_carry != 0) {
                if (!first_result_digit) {
                    output << ' ';
                }
                output << final_carry;
            }
            output << '\n';

            // Replace the reserved result length with the actual one.
            output.seekp(0);
            output << total_length + final_carry << '\n';
            output.flush();
            if (!output) {
                throw runtime_error("Failed while writing number3.txt.");
            }
        } else {
            const int workers = size - 1;
            int lengths[3];

            // Receive the lengths first so this worker can calculate its range.
            MPI_Recv(lengths, 3, MPI_INT, 0, TAG_LENGTHS,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            const int total_length = lengths[2];
            const int index = rank - 1;
            const int base = total_length / workers;
            const int remainder = total_length % workers;
            const int count = base + (index < remainder ? 1 : 0);

            unsigned char* first_digits = new unsigned char[count > 0 ? count : 1];
            unsigned char* second_digits = new unsigned char[count > 0 ? count : 1];

            // Receive both parts before receiving the previous worker's carry.
            MPI_Recv(first_digits, count, MPI_UNSIGNED_CHAR, 0,
                     TAG_FIRST_DIGITS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(second_digits, count, MPI_UNSIGNED_CHAR, 0,
                     TAG_SECOND_DIGITS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            unsigned char* result = new unsigned char[count + 1];
            int incoming_carry = 0;
            if (rank > 1) {
                if (!optimized) {
                    // Variant 1 waits for the previous worker's carry before
                    // it begins adding this segment.
                    MPI_Recv(&incoming_carry, 1, MPI_INT, rank - 1, TAG_CARRY,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }

            if (optimized && rank > 1) {
                unsigned char* result_if_zero = new unsigned char[count + 1];
                unsigned char* result_if_one = new unsigned char[count + 1];
                int carry_if_zero = 0;
                int carry_if_one = 1;

                // Extra computes both possible results before waiting for the
                // carry. Once it arrives, it selects the matching candidate.
                for (int i = 0; i < count; ++i) {
                    int sum = first_digits[i] + second_digits[i] + carry_if_zero;
                    result_if_zero[i] = static_cast<unsigned char>(sum % 10);
                    carry_if_zero = sum / 10;

                    sum = first_digits[i] + second_digits[i] + carry_if_one;
                    result_if_one[i] = static_cast<unsigned char>(sum % 10);
                    carry_if_one = sum / 10;
                }

                // Wait only after both candidate segments and carries are ready.
                MPI_Recv(&incoming_carry, 1, MPI_INT, rank - 1, TAG_CARRY,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (incoming_carry == 0) {
                    for (int i = 0; i < count; ++i) {
                        result[i] = result_if_zero[i];
                    }
                    incoming_carry = carry_if_zero;
                } else {
                    for (int i = 0; i < count; ++i) {
                        result[i] = result_if_one[i];
                    }
                    incoming_carry = carry_if_one;
                }

                delete[] result_if_zero;
                delete[] result_if_one;
            } else {
                // Variant 1 (and process 1 in Extra) adds only after its
                // incoming carry is known.
                int carry = incoming_carry;
                for (int i = 0; i < count; ++i) {
                    const int sum = first_digits[i] + second_digits[i] + carry;
                    result[i] = static_cast<unsigned char>(sum % 10);
                    carry = sum / 10;
                }
                incoming_carry = carry;
            }

            const int outgoing_carry = incoming_carry;

            delete[] first_digits;
            delete[] second_digits;

            if (rank < size - 1) {
                // Forward the carry before returning the result to process 0.
                SEND_MESSAGE(&outgoing_carry, 1, MPI_INT, rank + 1, TAG_CARRY,
                             MPI_COMM_WORLD);
            }

            // Append the carry-out to the segment for process 0. Only the
            // last segment's carry-out becomes a new most-significant digit.
            result[count] = static_cast<unsigned char>(outgoing_carry);
            SEND_MESSAGE(result, count + 1, MPI_UNSIGNED_CHAR, 0,
                         TAG_RESULT, MPI_COMM_WORLD);

            delete[] result;
        }
    } catch (const exception& error) {
        if (rank == 0) {
            cerr << "Error: " << error.what() << endl;
        }
        // Stop the whole job rather than leave another process blocked in MPI_Recv.
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}
