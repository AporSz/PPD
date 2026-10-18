package tasks;

import model.Matrix;

public class SequentialTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;

    public SequentialTask(Matrix F, Matrix C, Matrix V) {
        this.F = F;
        this.C = C;
        this.V = V;
    }

    @Override
    public void run() {
        for (int i = 0; i < this.F.getN(); i++)
        {
            for (int j = 0; j < this.F.getM(); j++) {
                int sum = Matrix.convolveAt(F, C, i, j);
                V.set(sum, i, j);
            }
        }
    }
}
