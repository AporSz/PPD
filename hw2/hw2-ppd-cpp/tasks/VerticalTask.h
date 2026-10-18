#ifndef HW2_PPD_CPP_VERTICALTASK_H
#define HW2_PPD_CPP_VERTICALTASK_H

#include "../model/Matrix.h"

class VerticalTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;
    int start;
    int end;

public:
    VerticalTask(Matrix* F, Matrix* C, Matrix* V, int start, int end)
        : F(F), C(C), V(V), start(start), end(end) {}

    void run() {
        for (int i = 0; i < this->F->getN(); i++) {
            for (int j = start; j < end; j++) {
                int sum = Matrix::convolveAt(F, C, i, j);
                V->set(sum, i, j);
            }
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_VERTICALTASK_H
