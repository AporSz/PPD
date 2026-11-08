#include "number_reader.h"

#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>

namespace {
    unsigned char* readNumber(const std::string& filename, std::size_t& digit_count) {
        std::ifstream input(filename.c_str());
        if (!input) {
            throw std::runtime_error("Unable to open input file: " + filename);
        }

        long long declared_count;
        if (!(input >> declared_count) || declared_count <= 0) {
            throw std::runtime_error("Invalid digit count in input file: " + filename);
        }

        if (static_cast<unsigned long long>(declared_count) >
            static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
            throw std::runtime_error("Digit count is too large in input file: " + filename);
        }
        digit_count = static_cast<std::size_t>(declared_count);
        std::unique_ptr<unsigned char[]> digits(new unsigned char[digit_count]);

        for (std::size_t i = 0; i < digit_count; ++i) {
            int digit;
            if (!(input >> digit) || digit < 0 || digit > 9) {
                throw std::runtime_error("Invalid or missing digit at index " +
                                         std::to_string(i) + " in input file: " + filename);
            }
            digits[i] = static_cast<unsigned char>(digit);
        }

        std::string extra;
        if (input >> extra) {
            throw std::runtime_error("Unexpected data after the digits in input file: " + filename);
        }

        return digits.release();
    }
}

InputNumbers::InputNumbers()
    : first(nullptr), first_size(0), second(nullptr), second_size(0) {}

InputNumbers::~InputNumbers() {
    delete[] first;
    delete[] second;
}

InputNumbers::InputNumbers(InputNumbers&& other) noexcept
    : first(other.first),
      first_size(other.first_size),
      second(other.second),
      second_size(other.second_size) {
    other.first = nullptr;
    other.first_size = 0;
    other.second = nullptr;
    other.second_size = 0;
}

InputNumbers& InputNumbers::operator=(InputNumbers&& other) noexcept {
    if (this != &other) {
        delete[] first;
        delete[] second;
        first = other.first;
        first_size = other.first_size;
        second = other.second;
        second_size = other.second_size;
        other.first = nullptr;
        other.first_size = 0;
        other.second = nullptr;
        other.second_size = 0;
    }
    return *this;
}

InputNumbers NumberReader::read(const std::string& first_file,
                                const std::string& second_file) {
    InputNumbers numbers;
    numbers.first = readNumber(first_file, numbers.first_size);
    numbers.second = readNumber(second_file, numbers.second_size);
    return numbers;
}
