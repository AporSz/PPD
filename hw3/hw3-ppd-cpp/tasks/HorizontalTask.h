#ifndef HW2_PPD_CPP_HORIZONTALTASK_H
#define HW2_PPD_CPP_HORIZONTALTASK_H

#include "../model/Matrix.h"
#include <vector>

class HorizontalTask {
private:
    Matrix* F;
    Matrix* C;
    int start;
    int end;
    vector<int> previous_row;
    vector<int> next_row;

public:
    HorizontalTask(Matrix* F, Matrix* C, int start, int end, const vector<int>& previous_row, const vector<int>& next_row)
        : F(F), C(C), start(start), end(end), previous_row(previous_row), next_row(next_row) {}

    void run() {
        int n = this->F->getN();
        int m = this->F->getM();

        for (int i = start; i < end; i++) {
            vector<int> computed(m);

            for (int j = 0; j < m; j++) {
                int sum = 0;

                if (j != 0) {
                    sum += previous_row[j - 1] * C->get(0, 0);
                    sum += F->get(i, j - 1) * C->get(1, 0);
                    if (i == end - 1) {
                        sum += next_row[j - 1] * C->get(2, 0);
                    } else {
                        sum += F->get(i + 1, j - 1) * C->get(2, 0);
                    }
                } else {
                    sum += previous_row[0] * C->get(0, 0);
                    sum += F->get(i, 0) * C->get(1, 0);
                    if (i == end - 1) {
                        sum += next_row[0] * C->get(2, 0);
                    } else {
                        sum += F->get(i + 1, 0) * C->get(2, 0);
                    }
                }

                sum += previous_row[j] * C->get(0, 1);
                sum += F->get(i, j) * C->get(1, 1);
                if (i == end - 1) {
                    sum += next_row[j] * C->get(2, 1);
                } else {
                    sum += F->get(i + 1, j) * C->get(2, 1);
                }

                if (j != m - 1) {
                    sum += previous_row[j + 1] * C->get(0, 2);
                    sum += F->get(i, j + 1) * C->get(1, 2);
                    if (i == end - 1) {
                        sum += next_row[j + 1] * C->get(2, 2);
                    } else {
                        sum += F->get(i + 1, j + 1) * C->get(2, 2);
                    }
                } else {
                    sum += previous_row[m - 1] * C->get(0, 2);
                    sum += F->get(i, m - 1) * C->get(1, 2);
                    if (i == end - 1) {
                        sum += next_row[m - 1] * C->get(2, 2);
                    } else {
                        sum += F->get(i + 1, m - 1) * C->get(2, 2);
                    }
                }

                computed[j] = sum;
            }
            previous_row = F->getRow(i);
            F->setRow(i, computed);
        }
    }

    void operator()() {
        run();
    }
};

#endif //HW2_PPD_CPP_HORIZONTALTASK_H
