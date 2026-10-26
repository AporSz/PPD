#ifndef HW2_PPD_CPP_HORIZONTALEXECUTOR_H
#define HW2_PPD_CPP_HORIZONTALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/HorizontalTask.h"

class HorizontalExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;

public:
    HorizontalExecutor(Matrix* F, Matrix* C) : F(F), C(C) {}

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
            int prev = start - 1;
            if (prev < 0) prev = 0;
            int next = end;
            if (next == n) next = n - 1;

            threads.emplace_back(HorizontalTask(this->F, this->C, start, end, this->F->getRow(prev), this->F->getRow(next)));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_HORIZONTALEXECUTOR_H
