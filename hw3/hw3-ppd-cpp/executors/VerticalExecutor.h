#ifndef HW2_PPD_CPP_VERTICALEXECUTOR_H
#define HW2_PPD_CPP_VERTICALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/VerticalTask.h"

class VerticalExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;

public:
    VerticalExecutor(Matrix* F, Matrix* C) : F(F), C(C) {}

    void executeTask(int p) override {
        int m = this->F->getM();
        int size = m / p;
        int extra = m % p;

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
            if (next == m) next = m - 1;

            threads.emplace_back(VerticalTask(this->F, this->C, start, end, this->F->getColumn(prev), this->F->getColumn(next)));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_VERTICALEXECUTOR_H
