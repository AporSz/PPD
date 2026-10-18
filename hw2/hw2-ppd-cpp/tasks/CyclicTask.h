#ifndef HW2_PPD_CPP_CYCLICTASK_H
#define HW2_PPD_CPP_CYCLICTASK_H

#include "../model/Matrix.h"

class CyclicTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;
    int id;
    int p;

public:
    CyclicTask(Matrix* F, Matrix* C, Matrix* V, int id, int p)
        : F(F), C(C), V(V), id(id), p(p) {}

    void run() {
        int n = this->F->getN();
        int m = this->F->getM();

        for (int k = id; k < n * m; k = k + p) {
            int i = k / m;
            int j = k % m;
            int sum = Matrix::convolveAt(F, C, i, j);
            V->set(sum, i, j);
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_CYCLICTASK_H
