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
        int size = m / p;
        int extra = m % p;

        Thread[] threads = new Thread[p];
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size + (id < extra ? 1 : 0);
            int previous = start > 0 ? start - 1 : 0;
            int next = end < m ? end : m - 1;
            threads[id] = new Thread(new VerticalTask(this.F, this.C, start, end,
                    F.getColumn(previous), F.getColumn(next)));
            start = end;
        }
        ConvolutionExecutor.startAndJoin(threads);
    }
}
