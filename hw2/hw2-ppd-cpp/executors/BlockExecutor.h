#ifndef HW2_PPD_CPP_BLOCKEXECUTOR_H
#define HW2_PPD_CPP_BLOCKEXECUTOR_H

#include "ConvolutionExecutor.h"
#include "../tasks/BlockTask.h"
#include <cmath>
#include <limits>

class BlockExecutor : public ConvolutionExecutor {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;

public:
    BlockExecutor(Matrix* F, Matrix* C, Matrix* V) : F(F), C(C), V(V) {}

    void executeTask(int p) override {
        int n = this->F->getN();
        int m = this->F->getM();
        double targetPr = sqrt((double) (p * n) / m);
        int pr = 1;
        double minDiff = numeric_limits<double>::max();
        for (int d = 1; d <= p; d++) {
            if (p % d == 0 && d <= n) {
                double diff = abs(d - targetPr);
                if (diff < minDiff) {
                    minDiff = diff;
                    pr = d;
                }
            }
        }
        int pc = p / pr;

        vector<thread> threads;
        for (int id = 0; id < p; id++) {
            threads.emplace_back(BlockTask(this->F, this->C, this->V, id, pr, pc));
        }
        ConvolutionExecutor::startAndJoin(threads);
    }
};

#endif //HW2_PPD_CPP_BLOCKEXECUTOR_H
