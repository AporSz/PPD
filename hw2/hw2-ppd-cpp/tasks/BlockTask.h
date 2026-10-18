#ifndef HW2_PPD_CPP_BLOCKTASK_H
#define HW2_PPD_CPP_BLOCKTASK_H

#include "../model/Matrix.h"
#include <algorithm>

using namespace std;

class BlockTask {
private:
    Matrix* F;
    Matrix* C;
    Matrix* V;
    int id;
    int pr;
    int pc;

public:
    BlockTask(Matrix* F, Matrix* C, Matrix* V, int id, int pr, int pc)
        : F(F), C(C), V(V), id(id), pr(pr), pc(pc) {}

    void run() {
        int n = this->F->getN();
        int m = this->F->getM();
        int baseHeight = n / pr;
        int rowRemainder = n % pr;
        int baseWidth = m / pc;
        int columnRemainder = m % pc;
        int tr = id / pc;
        int tc = id % pc;

        int startRow = tr * baseHeight + min(tr, rowRemainder);
        int endRow = startRow + baseHeight + (tr < rowRemainder ? 1 : 0);

        int startCol = tc * baseWidth + min(tc, columnRemainder);
        int endCol = startCol + baseWidth + (tc < columnRemainder ? 1 : 0);

        for (int i = startRow; i < endRow; i++) {
            for (int j = startCol; j < endCol; j++) {
                int sum = Matrix::convolveAt(this->F, this->C, i, j);
                V->set(sum, i, j);
            }
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_BLOCKTASK_H
