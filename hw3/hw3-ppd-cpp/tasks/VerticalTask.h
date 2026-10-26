#ifndef HW2_PPD_CPP_VERTICALTASK_H
#define HW2_PPD_CPP_VERTICALTASK_H

#include "../model/Matrix.h"
#include <vector>

class VerticalTask {
private:
    Matrix* F;
    Matrix* C;
    int start;
    int end;
    vector<int> previous_column;
    vector<int> next_column;

public:
    VerticalTask(Matrix* F, Matrix* C, int start, int end, const vector<int>& previous_column, const vector<int>& next_column)
        : F(F), C(C), start(start), end(end), previous_column(previous_column), next_column(next_column) {}

    void run() {
        int n = this->F->getN();
        int m = this->F->getM();

        for (int j = start; j < end; j++) {
            vector<int> computed(n);

            for (int i = 0; i < n; i++) {
                int sum = 0;

                if (i != 0) {
                    sum += previous_column[i - 1] * C->get(0, 0);
                    sum += F->get(i - 1, j) * C->get(0, 1);
                    if (j == end - 1) {
                        sum += next_column[i - 1] * C->get(0, 2);
                    } else {
                        sum += F->get(i - 1, j + 1) * C->get(0, 2);
                    }
                } else {
                    sum += previous_column[0] * C->get(0, 0);
                    sum += F->get(0, j) * C->get(0, 1);
                    if (j == end - 1) {
                        sum += next_column[0] * C->get(0, 2);
                    } else {
                        sum += F->get(0, j + 1) * C->get(0, 2);
                    }
                }

                sum += previous_column[i] * C->get(1, 0);
                sum += F->get(i, j) * C->get(1, 1);
                if (j == end - 1) {
                    sum += next_column[i] * C->get(1, 2);
                } else {
                    sum += F->get(i, j + 1) * C->get(1, 2);
                }

                if (i != n - 1) {
                    sum += previous_column[i + 1] * C->get(2, 0);
                    sum += F->get(i + 1, j) * C->get(2, 1);
                    if (j == end - 1) {
                        sum += next_column[i + 1] * C->get(2, 2);
                    } else {
                        sum += F->get(i + 1, j + 1) * C->get(2, 2);
                    }
                } else {
                    sum += previous_column[n - 1] * C->get(2, 0);
                    sum += F->get(n - 1, j) * C->get(2, 1);
                    if (j == end - 1) {
                        sum += next_column[n - 1] * C->get(2, 2);
                    } else {
                        sum += F->get(n - 1, j + 1) * C->get(2, 2);
                    }
                }

                computed[i] = sum;
            }
            previous_column = F->getColumn(j);
            F->setColumn(j, computed);
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_VERTICALTASK_H
