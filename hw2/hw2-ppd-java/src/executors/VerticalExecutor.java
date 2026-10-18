package executors;

import model.Matrix;
import tasks.VerticalTask;

public class VerticalExecutor implements ConvolutionExecutor {
    private Matrix F;
    private Matrix C;
    private Matrix V;

    public VerticalExecutor(Matrix F, Matrix C, Matrix V) {
        this.F = F;
        this.C = C;
        this.V = V;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        int m = this.F.getM();
        int size = m / p;    // every thread gets at least 'size' elements
        int extra = m % p;   // the first 'extra' threads get one more element

        Thread[] threads = new Thread[p];
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size;
            if (id < extra) {
                end++;
            }
            threads[id] = new Thread(new VerticalTask(this.F, this.C, this.V, start, end));
            start = end;
        }
        ConvolutionExecutor.startAndJoin(threads);
    }
}