//
// Created by szaka on 10/18/2026.
//

#ifndef HW2_PPD_CPP_STATICMATRIX_H
#define HW2_PPD_CPP_STATICMATRIX_H
#include "Matrix.h"


class StaticMatrix: public Matrix {
    private:
        int n, m;
        int a[10000][10000];
    public:
        StaticMatrix(int n, int m);
        int get(int i, int j) override;
        void set(int v, int i, int j) override;
        int getN() override;
        int getM() override;
        void reAllocate() override;
        Matrix fromFile(const string& filename) override;
        void toFile(const string& filename);
        string toString();
        int convolveAt(const Matrix& F, const Matrix& C, int i, int j);
        bool equals(const Matrix& other);
};


#endif //HW2_PPD_CPP_STATICMATRIX_H
