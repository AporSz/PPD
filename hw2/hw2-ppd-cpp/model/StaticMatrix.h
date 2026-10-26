#ifndef HW2_PPD_CPP_STATICMATRIX_H
#define HW2_PPD_CPP_STATICMATRIX_H

#include "Matrix.h"

class StaticMatrix : public Matrix {
private:
    static const int MAX_SIZE = 10000;
    static const int MATRIX_COUNT = 3;

    static int storage[MATRIX_COUNT][MAX_SIZE][MAX_SIZE];
    static bool inUse[MATRIX_COUNT];

    int n, m;
    int slot;

public:
    StaticMatrix(int n, int m);
    ~StaticMatrix() override;
    StaticMatrix(const StaticMatrix&) = delete;
    StaticMatrix& operator=(const StaticMatrix&) = delete;
    int get(int i, int j) override;
    void set(int v, int i, int j) override;
    int getN() override;
    int getM() override;
    void reAllocate() override;
};

#endif //HW2_PPD_CPP_STATICMATRIX_H
