#ifndef HW2_PPD_CPP_SEQUENTIALEXECUTOR_H
#define HW2_PPD_CPP_SEQUENTIALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/SequentialTask.h"

class SequentialExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    SequentialExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void executeTask(int p) override {
        SequentialTask task(this->F, this->C, this->V);
        task.run();
    }
};

#endif //HW2_PPD_CPP_SEQUENTIALEXECUTOR_H
