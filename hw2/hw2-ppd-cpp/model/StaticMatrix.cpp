//
// Created by szaka on 10/18/2026.
//

#include "StaticMatrix.h"

#include <fstream>
#include <stdexcept>

StaticMatrix::StaticMatrix(int n, int m) {
    this->n = n;
    this->m = m;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            this->a[i][j] = 0;
        }
    }
}

int StaticMatrix::get(int i, int j) {
    if (i < 0) {
        if (j < 0) return a[0][0];
        if (j >= this->m) return a[0][this->m-1];

        return a[0][j];
    }
    if (j < 0) {
        if (i >= this->n) return a[this->n-1][0];
        return a[i][0];
    }

    if (i >= this->n) {
        if (j >= this->m) return a[this->n-1][this->m-1];

        return a[this->n-1][j];
    }
    if (j >= this->m) {
        return a[i][this->m-1];
    }

    return a[i][j];
}

void StaticMatrix::set(int v, int i, int j) {
    if (i < 0 || i >= this->n || j < 0 || j >= this->m) throw std::out_of_range("Index out of range!");
    this->a[i][j] = v;
}

int StaticMatrix::getN() {
    return this->n;
}

int StaticMatrix::getM() {
    return this->m;
}

void StaticMatrix::reAllocate() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            this->a[i][j] = 0;
        }
    }
}