package tasks;

import model.Matrix;

public class VerticalTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private int start;
    private int end;
    private int[] previousColumn;
    private int[] nextColumn;

    public VerticalTask(Matrix F, Matrix C, int start, int end, int[] previousColumn, int[] nextColumn) {
        this.F = F;
        this.C = C;
        this.start = start;
        this.end = end;
        this.previousColumn = previousColumn;
        this.nextColumn = nextColumn;
    }

    @Override
    public void run() {
        int n = this.F.getN();

        for (int j = start; j < end; j++) {
            int[] computed = new int[n];

            for (int i = 0; i < n; i++) {
                int sum = 0;

                int above = i > 0 ? i - 1 : 0;
                int below = i < n - 1 ? i + 1 : n - 1;
                sum += previousColumn[above] * C.get(0, 0);
                sum += F.get(above, j) * C.get(0, 1);
                sum += (j == end - 1 ? nextColumn[above] : F.get(above, j + 1)) * C.get(0, 2);

                sum += previousColumn[i] * C.get(1, 0);
                sum += F.get(i, j) * C.get(1, 1);
                sum += (j == end - 1 ? nextColumn[i] : F.get(i, j + 1)) * C.get(1, 2);

                sum += previousColumn[below] * C.get(2, 0);
                sum += F.get(below, j) * C.get(2, 1);
                sum += (j == end - 1 ? nextColumn[below] : F.get(below, j + 1)) * C.get(2, 2);

                computed[i] = sum;
            }

            previousColumn = F.getColumn(j);
            F.setColumn(j, computed);
        }
    }
}
