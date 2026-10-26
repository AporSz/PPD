package tasks;

import model.Matrix;

public class HorizontalTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private int start;
    private int end;
    private int[] previous_row;
    private int[] next_row;

    public HorizontalTask(Matrix F, Matrix C, int start, int end, int[] previous_row, int[] next_row) {
        this.F = F;
        this.C = C;
        this.start = start;
        this.end = end;
        this.previous_row = previous_row;
        this.next_row = next_row;
    }

    @Override
    public void run() {
        int n = this.F.getN();
        int m = this.F.getM();

        for (int i = start; i < end; i++)
        {
            int[] computed = new int[m];

            for (int j = 0; j < m; j++) {
                int sum = 0;

                if (j != 0) {
                    sum += previous_row[j-1] * C.get(0, 0);
                    sum += F.get(i, j-1) * C.get(1, 0);
                    if (i == end - 1) {
                        sum += next_row[j-1] * C.get(2, 0);
                    }
                    else {
                        sum += F.get(i+1, j-1) * C.get(2, 0);
                    }
                }
                else {
                    sum += previous_row[0] * C.get(0, 0);
                    sum += F.get(i, 0) * C.get(1, 0);
                    if (i == end - 1) {
                        sum += next_row[0] * C.get(2, 0);
                    }
                    else {
                        sum += F.get(i+1, 0) * C.get(2, 0);
                    }
                }

                sum += previous_row[j] * C.get(0, 1);
                sum += F.get(i, j) * C.get(1, 1);
                if (i == end - 1) {
                    sum += next_row[j] * C.get(2, 1);
                }
                else {
                    sum += F.get(i+1, j) * C.get(2, 1);
                }

                if (j != m - 1) {
                    sum += previous_row[j+1] * C.get(0, 2);
                    sum += F.get(i, j+1) * C.get(1, 2);
                    if (i == end - 1) {
                        sum += next_row[j+1] * C.get(2, 2);
                    }
                    else {
                        sum += F.get(i+1, j+1) * C.get(2, 2);
                    }
                }
                else {
                    sum += previous_row[m-1] * C.get(0, 2);
                    sum += F.get(i, m-1) * C.get(1, 2);
                    if (i == end - 1) {
                        sum += next_row[m-1] * C.get(2, 2);
                    }
                    else {
                        sum += F.get(i+1, m-1) * C.get(2, 2);
                    }
                }

                computed[j] = sum;
            }
            previous_row = F.getRow(i);
            F.setRow(i, computed);
        }
    }
}
