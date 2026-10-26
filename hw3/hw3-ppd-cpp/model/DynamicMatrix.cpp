#include "DynamicMatrix.h"
#include <stdexcept>

DynamicMatrix::DynamicMatrix(int n, int m) {
    this->n = n;
    this->m = m;
    this->a = new int*[n];
    for (int i = 0; i < n; i++) {
        this->a[i] = new int[m]();
    }
}

DynamicMatrix::~DynamicMatrix() {
    if (this->a != nullptr) {
        for (int i = 0; i < this->n; i++) {
            delete[] this->a[i];
        }
        delete[] this->a;
    }
}

int DynamicMatrix::get(int i, int j) {
    if (i < 0) {
        if (j < 0) return a[0][0];
        if (j >= this->m) return a[0][this->m - 1];

        return a[0][j];
    }
    if (j < 0) {
        if (i >= this->n) return a[this->n - 1][0];
        return a[i][0];
    }

    if (i >= this->n) {
        if (j >= this->m) return a[this->n - 1][this->m - 1];

        return a[this->n - 1][j];
    }
    if (j >= this->m) {
        return a[i][this->m - 1];
    }

    return a[i][j];
}

void DynamicMatrix::set(int v, int i, int j) {
    if (i < 0 || i >= this->n || j < 0 || j >= this->m) throw std::out_of_range("Index out of range!");
    this->a[i][j] = v;
}

int DynamicMatrix::getN() {
    return this->n;
}

int DynamicMatrix::getM() {
    return this->m;
}

void DynamicMatrix::reAllocate() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            this->a[i][j] = 0;
        }
    }
}

vector<int> DynamicMatrix::getRow(int i) {
    if (i < 0 || i >= this->n) throw std::out_of_range("Index out of bounds");

    vector<int> row(m);
    for (int j = 0; j < m; j++) {
        row[j] = a[i][j];
    }
    return row;
}

void DynamicMatrix::setRow(int i, const vector<int>& row) {
    if (i < 0 || i >= this->n) throw std::out_of_range("Index out of bounds");

    for (int j = 0; j < m; j++) {
        a[i][j] = row[j];
    }
}

vector<int> DynamicMatrix::getColumn(int j) {
    if (j < 0 || j >= this->m) throw std::out_of_range("Index out of bounds");

    vector<int> column(n);
    for (int i = 0; i < n; i++) {
        column[i] = a[i][j];
    }
    return column;
}

void DynamicMatrix::setColumn(int j, const vector<int>& column) {
    if (j < 0 || j >= this->m) throw std::out_of_range("Index out of bounds");

    for (int i = 0; i < n; i++) {
        a[i][j] = column[i];
    }
}
