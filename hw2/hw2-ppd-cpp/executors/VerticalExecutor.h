#ifndef HW2_PPD_CPP_VERTICALEXECUTOR_H
#define HW2_PPD_CPP_VERTICALEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/VerticalTask.h"

class VerticalExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    VerticalExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

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
            threads.emplace_back(VerticalTask(this->F, this->C, this->V, start, end));
            start = end;
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_VERTICALEXECUTOR_H
