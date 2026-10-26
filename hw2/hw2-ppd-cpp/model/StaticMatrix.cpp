#include "StaticMatrix.h"
#include <algorithm>
#include <stdexcept>

int StaticMatrix::storage[StaticMatrix::MATRIX_COUNT][StaticMatrix::MAX_SIZE][StaticMatrix::MAX_SIZE] = {};
bool StaticMatrix::inUse[StaticMatrix::MATRIX_COUNT] = {};

StaticMatrix::StaticMatrix(int n, int m) : n(n), m(m), slot(-1) {
    if (n <= 0 || m <= 0 || n > MAX_SIZE || m > MAX_SIZE) {
        throw std::invalid_argument("Matrix dimensions must be between 1 and 10000 for static allocation.");
    }

    for (int i = 0; i < MATRIX_COUNT; i++) {
        if (!inUse[i]) {
            slot = i;
            inUse[i] = true;
            break;
        }
    }
    if (slot == -1) {
        throw std::runtime_error("No static matrix slots are available.");
    }

    reAllocate();
}

StaticMatrix::~StaticMatrix() {
    inUse[slot] = false;
}

int StaticMatrix::get(int i, int j) {
    if (i < 0) {
        i = 0;
    } else if (i >= this->n) {
        i = this->n - 1;
    }

    if (j < 0) {
        j = 0;
    } else if (j >= this->m) {
        j = this->m - 1;
    }

    return storage[slot][i][j];
}

void StaticMatrix::set(int v, int i, int j) {
    if (i < 0 || i >= this->n || j < 0 || j >= this->m) throw std::out_of_range("Index out of range!");
    storage[slot][i][j] = v;
}

int StaticMatrix::getN() {
    return this->n;
}

int StaticMatrix::getM() {
    return this->m;
}

void StaticMatrix::reAllocate() {
    for (int i = 0; i < n; i++) {
        std::fill(storage[slot][i], storage[slot][i] + m, 0);
    }
}