//
// Created by apor on 11/8/26.
//

#include "Sequential.h"

#include <algorithm>

unsigned char* Sequential::add(unsigned char* a, int na, unsigned char* b, int nb) {
    int n = std::max(na, nb) + 1;
    unsigned char* c = new unsigned char[n];
    bool carry = false;

    for (int i = 0; i < n; i++) {
        if (i == n - 1) c[i] = carry;
        else {
            if (i >= na) {
                c[i] = b[i] + carry;
                if (c[i] > 9) {
                    c[i] = c[i] - 10;
                    carry = true;
                }
                else carry = false;
            }
            else {
                if (i >= nb) {
                    c[i] = a[i] + carry;
                    if (c[i] > 9 ) {
                        c[i] = c[i] - 10;
                        carry = true;
                    }
                    else carry = false;
                }
                else {
                    c[i] = a[i] + b[i] + carry;
                    if (c[i] > 9) {
                        c[i] = c[i] - 10;
                        carry = true;
                    }
                    else carry = false;
                }
            }
        }
    }

    return c;
}
