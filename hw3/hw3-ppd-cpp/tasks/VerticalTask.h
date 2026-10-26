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

        for (int j = start; j < end; j++) {
            vector<int> computed(n);

            for (int i = 0; i < n; i++) {
                int sum = 0;

                int above = i > 0 ? i - 1 : 0;
                int below = i < n - 1 ? i + 1 : n - 1;
                sum += previous_column[above] * C->get(0, 0);
                sum += F->get(above, j) * C->get(0, 1);
                sum += (j == end - 1 ? next_column[above] : F->get(above, j + 1)) * C->get(0, 2);

                sum += previous_column[i] * C->get(1, 0);
                sum += F->get(i, j) * C->get(1, 1);
                sum += (j == end - 1 ? next_column[i] : F->get(i, j + 1)) * C->get(1, 2);

                sum += previous_column[below] * C->get(2, 0);
                sum += F->get(below, j) * C->get(2, 1);
                sum += (j == end - 1 ? next_column[below] : F->get(below, j + 1)) * C->get(2, 2);

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
