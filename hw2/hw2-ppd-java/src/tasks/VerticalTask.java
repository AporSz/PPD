package tasks;

import model.Matrix;

public class VerticalTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;
    private int start;
    private int end;

    public VerticalTask(Matrix F, Matrix C, Matrix V, int start, int end) {
        this.F = F;
        this.C = C;
        this.V = V;
        this.start = start;
        this.end = end;
    }

    @Override
    public void run() {
        for (int i = 0; i < this.F.getN(); i++)
        {
            for (int j = start; j < end; j++) {
                int sum = Matrix.convolveAt(F, C, i, j);
                V.set(sum, i, j);
            }
        }
    }
}