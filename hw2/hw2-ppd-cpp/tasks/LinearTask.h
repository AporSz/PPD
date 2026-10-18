#ifndef HW2_PPD_CPP_LINEARTASK_H
#define HW2_PPD_CPP_LINEARTASK_H

#include "../model/Matrix.h"

class LinearTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;
    int start;
    int end;

public:
    LinearTask(Matrix* F, Matrix* C, Matrix* V, int start, int end)
        : F(F), C(C), V(V), start(start), end(end) {}

    void run() {
        int m = this->F->getM();

        for (int k = start; k < end; k++) {
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

#endif //HW2_PPD_CPP_LINEARTASK_H
