package tasks;

import model.Matrix;

public class VerticalTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private int start;
    private int end;
    private int[] previous_column;
    private int[] next_column;

    public VerticalTask(Matrix F, Matrix C, int start, int end, int[] previous_column, int[] next_column) {
        this.F = F;
        this.C = C;
        this.start = start;
        this.end = end;
        this.previous_column = previous_column;
        this.next_column = next_column;
    }

    @Override
    public void run() {
        int n = this.F.getN();
        int m = this.F.getM();

        for (int j = start; j < end; j++)
        {
            int[] computed = new int[n];

            for (int i = 0; i < n; i++) {
                int sum = 0;

                if (i != 0) {
                    sum += previous_column[i-1] * C.get(0, 0);
                    sum += F.get(i-1, j) * C.get(0, 1);
                    if (j == end - 1) {
                        sum += next_column[i-1] * C.get(0, 2);
                    }
                    else {
                        sum += F.get(i-1, j+1) * C.get(0, 2);
                    }
                }
                else {
                    sum += previous_column[0] * C.get(0, 0);
                    sum += F.get(0, j) * C.get(0, 1);
                    if (j == end - 1) {
                        sum += next_column[0] * C.get(0, 2);
                    }
                    else {
                        sum += F.get(0, j+1) * C.get(0, 2);
                    }
                }

                sum += previous_column[i] * C.get(1, 0);
                sum += F.get(i, j) * C.get(1, 1);
                if (j == end - 1) {
                    sum += next_column[i] * C.get(1, 2);
                }
                else {
                    sum += F.get(i, j+1) * C.get(1, 2);
                }

                if (i != n - 1) {
                    sum += previous_column[i+1] * C.get(2, 0);
                    sum += F.get(i+1, j) * C.get(2, 1);
                    if (j == end - 1) {
                        sum += next_column[i+1] * C.get(2, 2);
                    }
                    else {
                        sum += F.get(i+1, j+1) * C.get(2, 2);
                    }
                }
                else {
                    sum += previous_column[n-1] * C.get(2, 0);
                    sum += F.get(n-1, j) * C.get(2, 1);
                    if (j == end - 1) {
                        sum += next_column[n-1] * C.get(2, 2);
                    }
                    else {
                        sum += F.get(n-1, j+1) * C.get(2, 2);
                    }
                }

                computed[i] = sum;
            }
            previous_column = F.getColumn(j);
            F.setColumn(j, computed);
        }
    }
}