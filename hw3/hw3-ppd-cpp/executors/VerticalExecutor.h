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
            int end = start + size + (id < extra ? 1 : 0);
            int previous = start > 0 ? start - 1 : 0;
            int next = end < m ? end : m - 1;
            threads.emplace_back(VerticalTask(this->F, this->C, start, end,
                                              this->F->getColumn(previous),
                                              this->F->getColumn(next)));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_VERTICALEXECUTOR_H
