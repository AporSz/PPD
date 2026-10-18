#ifndef HW2_PPD_CPP_LINEAREXECUTOR_H
#define HW2_PPD_CPP_LINEAREXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/LinearTask.h"

class LinearExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    LinearExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void executeTask(int p) override {
        int n = this->F->getN() * this->F->getM();
        int size = n / p;
        int extra = n % p;

        vector<thread> threads;
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size;
            if (id < extra) {
                end++;
            }
            threads.emplace_back(LinearTask(this->F, this->C, this->V, start, end));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_LINEAREXECUTOR_H
