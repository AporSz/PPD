#ifndef HW4_NUMBER_READER_H
#define HW4_NUMBER_READER_H

#include <cstddef>
#include <string>

struct InputNumbers {
    // Both arrays preserve file order, with index 0 as the least-significant digit.
    unsigned char* first;
    std::size_t first_size;
    unsigned char* second;
    std::size_t second_size;

    InputNumbers();
    ~InputNumbers();

    InputNumbers(const InputNumbers&) = delete;
    InputNumbers& operator=(const InputNumbers&) = delete;
    InputNumbers(InputNumbers&& other) noexcept;
    InputNumbers& operator=(InputNumbers&& other) noexcept;
};

class NumberReader {
public:
    static InputNumbers read(const std::string& first_file = "../number1.txt",
                             const std::string& second_file = "../number2.txt");
};

#endif
