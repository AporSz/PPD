#ifndef HW4_NUMBER_WRITER_H
#define HW4_NUMBER_WRITER_H

#include <cstddef>
#include <string>

class NumberWriter {
public:
    static void write(const unsigned char* digits,
                      std::size_t digit_count,
                      const std::string& filename = "/home/apor/Documents/My_Things/UNI/PPD/hw4/number3.txt");
};

#endif
