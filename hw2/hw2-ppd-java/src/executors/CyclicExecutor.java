package executors;

import model.Matrix;
import tasks.CyclicTask;

public class CyclicExecutor implements ConvolutionExecutor {
    private Matrix F;
    private Matrix C;
    private Matrix V;

    public CyclicExecutor(Matrix F, Matrix C, Matrix V) {
        this.F = F;
        this.C = C;
        this.V = V;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        Thread[] threads = new Thread[p];

        for (int i=0; i<p; i++) {
            threads[i] = new Thread(new CyclicTask(this.F, this.C, this.V, i, p));
        }

        ConvolutionExecutor.startAndJoin(threads);
    }
}
