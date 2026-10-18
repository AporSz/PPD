//
// Created by apor on 10/18/26.
//

#ifndef HW2_PPD_CPP_MATRIX_H
#define HW2_PPD_CPP_MATRIX_H
#include <string>

using namespace std;

class Matrix {
public:
    virtual ~Matrix() = default;
    virtual int get(int i, int j);
    virtual void set(int v, int i, int j);
    virtual int getN();
    virtual int getM();
    virtual void reAllocate();
    virtual Matrix fromFile(const string& filename);
    virtual void toFile(const string& filename);
    virtual string toString();
    virtual int convolveAt(const Matrix& F, const Matrix& C, int i, int j);
    virtual bool equals(const Matrix& other);
};


#endif //HW2_PPD_CPP_MATRIX_H
