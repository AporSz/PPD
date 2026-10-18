#ifndef HW2_PPD_CPP_HORIZONTALEXECUTOR_H
#define HW2_PPD_CPP_HORIZONTALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/HorizontalTask.h"

class HorizontalExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    HorizontalExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void executeTask(int p) override {
        int n = this->F->getN();
        int size = n / p;
        int extra = n % p;

        vector<thread> threads;
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size;
            if (id < extra) {
                end++;
            }
            threads.emplace_back(HorizontalTask(this->F, this->C, this->V, start, end));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_HORIZONTALEXECUTOR_H
