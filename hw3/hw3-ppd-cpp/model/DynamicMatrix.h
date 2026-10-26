#ifndef HW2_PPD_CPP_DYNAMICMATRIX_H
#define HW2_PPD_CPP_DYNAMICMATRIX_H

#include "Matrix.h"
#include <vector>

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
    vector<int> getRow(int i) override;
    void setRow(int i, const vector<int>& row) override;
    vector<int> getColumn(int j) override;
    void setColumn(int j, const vector<int>& column) override;
};

#endif //HW2_PPD_CPP_DYNAMICMATRIX_H
