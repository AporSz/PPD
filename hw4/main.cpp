#include <iostream>

#include "solutions/Sequential.h"
#include "utils/number_reader.h"
#include "utils/number_writer.h"

int main() {

    unsigned char* a;
    unsigned char* b;
    unsigned char* c;

    InputNumbers numbers = NumberReader::read();

    a = numbers.first;
    int an = numbers.first_size;
    b = numbers.second;
    int bn = numbers.second_size;

    c = Sequential::add(a, an, b, bn);
    for (int i = 0; i < std::max(an, bn) + (c[std::max(an, bn)] != 0); i++) {
        std::cout << static_cast<int>(c[i]) << std::endl;
    }

    NumberWriter::write(c, std::max(an, bn) + (c[std::max(an, bn)] != 0));

    // delete[] a;
    // delete[] b;
    // delete[] c;

    return 0;
}
