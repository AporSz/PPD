#ifndef HW2_PPD_CPP_SEQUENTIALEXECUTOR_H
#define HW2_PPD_CPP_SEQUENTIALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/SequentialTask.h"

class SequentialExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;

public:
    SequentialExecutor(Matrix* F, Matrix* C) : F(F), C(C) {}

    void executeTask(int p) override {
        SequentialTask task(this->F, this->C);
        task.run();
    }
};

#endif //HW2_PPD_CPP_SEQUENTIALEXECUTOR_H
