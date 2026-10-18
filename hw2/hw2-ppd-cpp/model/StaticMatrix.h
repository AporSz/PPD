#ifndef HW2_PPD_CPP_STATICMATRIX_H
#define HW2_PPD_CPP_STATICMATRIX_H

#include "Matrix.h"
#include <vector>

class StaticMatrix : public Matrix {
private:
    int n, m;
    std::vector<int> a;

public:
    StaticMatrix(int n, int m);
    ~StaticMatrix() override = default;
    int get(int i, int j) override;
    void set(int v, int i, int j) override;
    int getN() override;
    int getM() override;
    void reAllocate() override;
};

#endif //HW2_PPD_CPP_STATICMATRIX_H
