package tasks;

import model.Matrix;

public class CyclicTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;
    private int id;
    private int p;

    public CyclicTask(Matrix F, Matrix C, Matrix V, int id, int p) {
        this.F = F;
        this.C = C;
        this.V = V;
        this.id = id;
        this.p = p;
    }

    @Override
    public void run() {
        int n = this.F.getN();
        int m = this.F.getM();

        for (int k = id; k < n * m; k = k + p)
        {
            int i = k / m;
            int j = k % m;
            int sum = Matrix.convolveAt(F, C, i, j);
            V.set(sum, i, j);
        }
    }
}
