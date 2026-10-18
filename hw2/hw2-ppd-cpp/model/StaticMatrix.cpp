#include "StaticMatrix.h"
#include <algorithm>
#include <stdexcept>

StaticMatrix::StaticMatrix(int n, int m) : n(n), m(m), a(static_cast<size_t>(n) * static_cast<size_t>(m), 0) {
    if (n <= 0 || m <= 0) {
        throw std::invalid_argument("Matrix dimensions must be positive.");
    }
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

    return this->a[static_cast<size_t>(i) * static_cast<size_t>(this->m) + static_cast<size_t>(j)];
}

void StaticMatrix::set(int v, int i, int j) {
    if (i < 0 || i >= this->n || j < 0 || j >= this->m) throw std::out_of_range("Index out of range!");
    this->a[static_cast<size_t>(i) * static_cast<size_t>(this->m) + static_cast<size_t>(j)] = v;
}

int StaticMatrix::getN() {
    return this->n;
}

int StaticMatrix::getM() {
    return this->m;
}

void StaticMatrix::reAllocate() {
    std::fill(this->a.begin(), this->a.end(), 0);
}