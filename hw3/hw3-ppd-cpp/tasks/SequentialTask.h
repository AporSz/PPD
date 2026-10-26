#ifndef HW2_PPD_CPP_SEQUENTIALTASK_H
#define HW2_PPD_CPP_SEQUENTIALTASK_H

#include "../model/Matrix.h"
#include <vector>

class SequentialTask {
private:
    Matrix* F;
    Matrix* C;

public:
    SequentialTask(Matrix* F, Matrix* C) : F(F), C(C) {}

    void run() {
        int m = this->F->getM();
        int n = this->F->getN();
        vector<int> prev = this->F->getRow(0);
        vector<int> current = this->F->getRow(0);
        vector<int> next = this->F->getRow(n > 1 ? 1 : 0);

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int sum = 0;
                if (j != 0) {
                    sum += prev[j - 1] * C->get(0, 0);
                    sum += current[j - 1] * C->get(1, 0);
                    sum += next[j - 1] * C->get(2, 0);
                } else {
                    sum += prev[0] * C->get(0, 0);
                    sum += current[0] * C->get(1, 0);
                    sum += next[0] * C->get(2, 0);
                }

                sum += prev[j] * C->get(0, 1);
                sum += current[j] * C->get(1, 1);
                sum += next[j] * C->get(2, 1);

                if (j != m - 1) {
                    sum += prev[j + 1] * C->get(0, 2);
                    sum += current[j + 1] * C->get(1, 2);
                    sum += next[j + 1] * C->get(2, 2);
                } else {
                    sum += prev[m - 1] * C->get(0, 2);
                    sum += current[m - 1] * C->get(1, 2);
                    sum += next[m - 1] * C->get(2, 2);
                }

                F->set(sum, i, j);
            }
            prev = current;
            current = next;
            if (i < n - 2) {
                next = this->F->getRow(i + 2);
            } else if (i == n - 1) {
                next = this->F->getRow(n - 1);
            } else {
                next = this->F->getRow(i + 1);
            }
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_SEQUENTIALTASK_H
