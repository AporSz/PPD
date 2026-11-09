#include "AsynchronousCommunication.h"

#include <mpi.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../utils/number_writer.h"

using namespace std;

void AsynchronousCommunication::add() {
    const int TAG_FIRST_DIGITS = 1;
    const int TAG_SECOND_DIGITS = 2;
    const int TAG_CARRY = 3;
    const int TAG_RESULT = 4;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            cerr << "Variant 3 requires at least two processes.\n";
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    int lengths[3] = {0, 0, 0};
    if (rank == 0) {
        ifstream first_input("../number1.txt");
        ifstream second_input("../number2.txt");
        if (!first_input || !second_input) {
            cerr << "Error: unable to open number1.txt or number2.txt.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return;
        }

        if (!(first_input >> lengths[0]) || !(second_input >> lengths[1]) ||
            lengths[0] <= 0 || lengths[1] <= 0) {
            cerr << "Error: invalid digit count in input file.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return;
        }
        lengths[2] = lengths[0] > lengths[1] ? lengths[0] : lengths[1];
    }

    // MPI_Bcast gives every process the input sizes before it posts receives.
    MPI_Bcast(lengths, 3, MPI_INT, 0, MPI_COMM_WORLD);

    const int workers = size - 1;
    const int total_length = lengths[2];
    const int chunk = total_length / workers + (total_length % workers != 0);
    const int padded_length = chunk * workers;
    const int result_stride = chunk + 1;

    unsigned char* first_digits = nullptr;
    unsigned char* second_digits = nullptr;
    unsigned char* received_results = nullptr;
    MPI_Request* input_requests = nullptr;
    MPI_Request* result_requests = nullptr;

    try {
        if (rank == 0) {
            first_digits = new unsigned char[padded_length]();
            second_digits = new unsigned char[padded_length]();
            received_results = new unsigned char[workers * result_stride]();
            input_requests = new MPI_Request[2 * workers];
            result_requests = new MPI_Request[workers];

            // Post all result receives before workers send. MPI_Irecv returns
            // immediately; MPI_Waitall below completes the received messages.
            for (int worker = 1; worker < size; ++worker) {
                const int result_offset = (worker - 1) * result_stride;
                MPI_Irecv(received_results + result_offset, result_stride,
                          MPI_UNSIGNED_CHAR, worker, TAG_RESULT, MPI_COMM_WORLD,
                          &result_requests[worker - 1]);
            }

            ifstream first_input("../number1.txt");
            ifstream second_input("../number2.txt");
            if (!first_input || !second_input) {
                throw runtime_error("Unable to reopen number1.txt or number2.txt.");
            }
            int ignored_length;
            first_input >> ignored_length;
            second_input >> ignored_length;

            int first_read = 0;
            int second_read = 0;
            for (int worker = 1; worker < size; ++worker) {
                const int index = worker - 1;
                const int offset = index * chunk;
                unsigned char* first_part = first_digits + offset;
                unsigned char* second_part = second_digits + offset;

                // Read one worker's chunk at a time, keeping input order
                // least-significant digit first. The buffers stay alive until
                // MPI_Waitall completes their nonblocking sends.
                for (int i = 0; i < chunk; ++i) {
                    const int digit_index = offset + i;
                    if (digit_index < lengths[0]) {
                        int digit;
                        if (!(first_input >> digit) || digit < 0 || digit > 9) {
                            throw runtime_error("Invalid or missing digit in number1.txt.");
                        }
                        first_part[i] = static_cast<unsigned char>(digit);
                        ++first_read;
                    }
                    if (digit_index < lengths[1]) {
                        int digit;
                        if (!(second_input >> digit) || digit < 0 || digit > 9) {
                            throw runtime_error("Invalid or missing digit in number2.txt.");
                        }
                        second_part[i] = static_cast<unsigned char>(digit);
                        ++second_read;
                    }
                }

                // MPI_Isend starts each digit transfer without waiting for its
                // matching receive. Keep each send buffer unchanged until done.
                MPI_Isend(first_part, chunk, MPI_UNSIGNED_CHAR, worker,
                          TAG_FIRST_DIGITS, MPI_COMM_WORLD,
                          &input_requests[2 * index]);
                MPI_Isend(second_part, chunk, MPI_UNSIGNED_CHAR, worker,
                          TAG_SECOND_DIGITS, MPI_COMM_WORLD,
                          &input_requests[2 * index + 1]);
            }

            string extra;
            if (first_read != lengths[0] || second_read != lengths[1] ||
                (first_input >> extra) || (second_input >> extra)) {
                throw runtime_error("Input digit count does not match its header.");
            }

            // MPI_Waitall guarantees that all input buffers may be reused and
            // that every asynchronous result receive has completed.
            MPI_Waitall(2 * workers, input_requests, MPI_STATUSES_IGNORE);
            MPI_Waitall(workers, result_requests, MPI_STATUSES_IGNORE);

            unsigned char* result = new unsigned char[total_length + 1]();
            int output_offset = 0;
            int final_carry = 0;
            for (int worker = 1; worker < size; ++worker) {
                const int index = worker - 1;
                const int result_offset = index * result_stride;
                const int digits_here =
                    total_length - output_offset < chunk
                        ? total_length - output_offset
                        : chunk;
                for (int i = 0; i < digits_here; ++i) {
                    result[output_offset + i] = received_results[result_offset + i];
                }
                output_offset += digits_here;
                if (worker == size - 1) {
                    final_carry = received_results[result_offset + chunk];
                }
            }

            result[total_length] = static_cast<unsigned char>(final_carry);
            NumberWriter::write(result, total_length + final_carry);
            delete[] result;
        } else {
            const int index = rank - 1;
            const int offset = index * chunk;
            int digits_here = total_length - offset;
            if (digits_here > chunk) {
                digits_here = chunk;
            }

            unsigned char* first_part = new unsigned char[chunk];
            unsigned char* second_part = new unsigned char[chunk];
            unsigned char* result_part = new unsigned char[result_stride]();
            int incoming_carry = 0;
            int outgoing_carry = 0;

            MPI_Request receive_requests[3];
            int receive_count = 0;

            // Post nonblocking receives for digits and carry together because
            // the carry and process-0 digit messages can arrive in either order.
            MPI_Irecv(first_part, chunk, MPI_UNSIGNED_CHAR, 0, TAG_FIRST_DIGITS,
                      MPI_COMM_WORLD, &receive_requests[receive_count++]);
            MPI_Irecv(second_part, chunk, MPI_UNSIGNED_CHAR, 0, TAG_SECOND_DIGITS,
                      MPI_COMM_WORLD, &receive_requests[receive_count++]);
            if (rank > 1) {
                MPI_Irecv(&incoming_carry, 1, MPI_INT, rank - 1, TAG_CARRY,
                          MPI_COMM_WORLD, &receive_requests[receive_count++]);
            }

            // MPI_Waitall completes all posted receives before the digits are
            // added, independent of which message arrived first.
            MPI_Waitall(receive_count, receive_requests, MPI_STATUSES_IGNORE);

            int carry = incoming_carry;
            for (int i = 0; i < digits_here; ++i) {
                const int sum = first_part[i] + second_part[i] + carry;
                result_part[i] = static_cast<unsigned char>(sum % 10);
                carry = sum / 10;
            }
            outgoing_carry = carry;
            result_part[chunk] = static_cast<unsigned char>(outgoing_carry);

            MPI_Request send_requests[2];
            int send_count = 0;

            // Forward the carry asynchronously. The last process has no next
            // worker, but its carry is included with its result message.
            if (rank < size - 1) {
                MPI_Isend(&outgoing_carry, 1, MPI_INT, rank + 1, TAG_CARRY,
                          MPI_COMM_WORLD, &send_requests[send_count++]);
            }

            // Return this worker's result asynchronously to process 0.
            MPI_Isend(result_part, result_stride, MPI_UNSIGNED_CHAR, 0,
                      TAG_RESULT, MPI_COMM_WORLD, &send_requests[send_count++]);

            // Keep the carry and result buffers valid until both sends finish.
            MPI_Waitall(send_count, send_requests, MPI_STATUSES_IGNORE);

            delete[] first_part;
            delete[] second_part;
            delete[] result_part;
        }

        delete[] first_digits;
        delete[] second_digits;
        delete[] received_results;
        delete[] input_requests;
        delete[] result_requests;
    } catch (const exception& error) {
        if (rank == 0) {
            cerr << "Error: " << error.what() << endl;
        }

        delete[] first_digits;
        delete[] second_digits;
        delete[] received_results;
        delete[] input_requests;
        delete[] result_requests;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}
