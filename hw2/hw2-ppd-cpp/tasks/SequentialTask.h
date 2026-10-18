#ifndef HW2_PPD_CPP_SEQUENTIALTASK_H
#define HW2_PPD_CPP_SEQUENTIALTASK_H

#include "../model/Matrix.h"

class SequentialTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    SequentialTask(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void run() {
        for (int i = 0; i < this->F->getN(); i++) {
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

#endif //HW2_PPD_CPP_SEQUENTIALTASK_H
