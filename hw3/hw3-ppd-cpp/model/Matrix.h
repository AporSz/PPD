#ifndef HW2_PPD_CPP_MATRIX_H
#define HW2_PPD_CPP_MATRIX_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class Matrix {
public:
    virtual ~Matrix() = default;
    virtual int get(int i, int j) = 0;
    virtual void set(int v, int i, int j) = 0;
    virtual int getN() = 0;
    virtual int getM() = 0;
    virtual void reAllocate() = 0;
    virtual void toFile(const string& filename);
    virtual string toString();
    virtual bool equals(Matrix* other);
    virtual vector<int> getRow(int i) = 0;
    virtual void setRow(int i, const vector<int>& row) = 0;
    virtual vector<int> getColumn(int j) = 0;
    virtual void setColumn(int j, const vector<int>& column) = 0;

    static int convolveAt(Matrix* F, Matrix* C, int i, int j);
    static Matrix* fromFile(const string& filename);
};

#endif //HW2_PPD_CPP_MATRIX_H
