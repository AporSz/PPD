#ifndef HW4_NUMBER_WRITER_H
#define HW4_NUMBER_WRITER_H

#include <cstddef>
#include <string>

class NumberWriter {
public:
    static void write(const unsigned char* digits,
                      std::size_t digit_count,
                      const std::string& filename = "../number3.txt");
};

#endif
