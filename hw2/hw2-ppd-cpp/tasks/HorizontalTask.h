#ifndef HW2_PPD_CPP_HORIZONTALTASK_H
#define HW2_PPD_CPP_HORIZONTALTASK_H

#include "../model/Matrix.h"

class HorizontalTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;
    int start;
    int end;

public:
    HorizontalTask(Matrix* F, Matrix* C, Matrix* V, int start, int end)
        : F(F), C(C), V(V), start(start), end(end) {}

    void run() {
        for (int i = start; i < end; i++) {
            for (int j = 0; j < this->F->getM(); j++) {
                int sum = Matrix::convolveAt(F, C, i, j);
                V->set(sum, i, j);
            }
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_HORIZONTALTASK_H
