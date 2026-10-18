#ifndef HW2_PPD_CPP_DYNAMICMATRIX_H
#define HW2_PPD_CPP_DYNAMICMATRIX_H

#include "Matrix.h"

class DynamicMatrix : public Matrix {
private:
    int n, m;
    int** a;

public:
    DynamicMatrix(int n, int m);
    ~DynamicMatrix() override;
    int get(int i, int j) override;
    void set(int v, int i, int j) override;
    int getN() override;
    int getM() override;
    void reAllocate() override;
};

#endif //HW2_PPD_CPP_DYNAMICMATRIX_H
