#include "number_writer.h"

#include <fstream>
#include <stdexcept>

void NumberWriter::write(const unsigned char* digits,
                         std::size_t digit_count,
                         const std::string& filename) {
    if (digits == nullptr || digit_count == 0) {
        throw std::invalid_argument("Result digits must not be null or empty.");
    }

    std::ofstream output(filename.c_str());
    if (!output) {
        throw std::runtime_error("Unable to open output file: " + filename);
    }

    output << digit_count << '\n';
    for (std::size_t i = 0; i < digit_count; ++i) {
        if (digits[i] > 9) {
            throw std::invalid_argument("Result contains an invalid digit at index " +
                                        std::to_string(i) + ".");
        }
        if (i != 0) {
            output << ' ';
        }
        output << static_cast<unsigned int>(digits[i]);
    }
    output << '\n';
    output.flush();

    if (!output) {
        throw std::runtime_error("Failed while writing output file: " + filename);
    }
}
