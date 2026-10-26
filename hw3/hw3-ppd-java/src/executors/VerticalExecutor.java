package executors;

import model.Matrix;
import tasks.VerticalTask;

public class VerticalExecutor implements ConvolutionExecutor {
    private Matrix F;
    private Matrix C;

    public VerticalExecutor(Matrix F, Matrix C) {
        this.F = F;
        this.C = C;
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
            int prev = start - 1;
            if (prev < 0) prev = 0;
            int next = end;
            if (next == m) next = m - 1;
            threads[id] = new Thread(new VerticalTask(this.F, this.C, start, end, this.F.getColumn(prev), this.F.getColumn(next)));
            start = end;
        }
        ConvolutionExecutor.startAndJoin(threads);
    }
}