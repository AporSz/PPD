package tasks;

import model.Matrix;

public class LinearTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;
    private int start;
    private int end;

    public LinearTask(Matrix F, Matrix C, Matrix V, int start, int end) {
        this.F = F;
        this.C = C;
        this.V = V;
        this.start = start;
        this.end = end;
    }

    @Override
    public void run() {
        int m = this.F.getM();

        for (int k = start; k < end; k++)
        {
            int i = k / m;
            int j = k % m;
            int sum = Matrix.convolveAt(F, C, i, j);
            V.set(sum, i, j);
        }
    }
}
