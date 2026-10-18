#ifndef HW2_PPD_CPP_CYCLICEXECUTOR_H
#define HW2_PPD_CPP_CYCLICEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/CyclicTask.h"

class CyclicExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    CyclicExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void executeTask(int p) override {
        vector<thread> threads;
        for (int i = 0; i < p; i++) {
            threads.emplace_back(CyclicTask(this->F, this->C, this->V, i, p));
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_CYCLICEXECUTOR_H
