package tasks;

import model.Matrix;

public class HorizontalTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;
    private int start;
    private int end;

    public HorizontalTask(Matrix F, Matrix C, Matrix V, int start, int end) {
        this.F = F;
        this.C = C;
        this.V = V;
        this.start = start;
        this.end = end;
    }

    @Override
    public void run() {
        for (int i = start; i < end; i++)
        {
            for (int j = 0; j < this.F.getM(); j++) {
                int sum = Matrix.convolveAt(F, C, i, j);
                V.set(sum, i, j);
            }
        }
    }
}
