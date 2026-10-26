package executors;

import tasks.HorizontalTask;
import model.Matrix;

public class HorizontalExecutor implements ConvolutionExecutor {
    private Matrix F;
    private Matrix C;

    public HorizontalExecutor(Matrix F, Matrix C) {
        this.F = F;
        this.C = C;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        int n = this.F.getN();
        int size = n / p;    // every thread gets at least 'size' elements
        int extra = n % p;   // the first 'extra' threads get one more element

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
            if (next == n) next = n - 1;
            threads[id] = new Thread(new HorizontalTask(this.F, this.C, start, end, F.getRow(prev), F.getRow(next)));
            start = end;
        }
        ConvolutionExecutor.startAndJoin(threads);
    }
}
