package executors;

import model.Matrix;
import tasks.BlockTask;

import static java.lang.Math.sqrt;

public class BlockExecutor implements ConvolutionExecutor {
    private Matrix F;
    private Matrix C;
    private Matrix V;

    public BlockExecutor(Matrix F, Matrix C, Matrix V) {
        this.F = F;
        this.C = C;
        this.V = V;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        int n = this.F.getN();
        int m = this.F.getM();
        double targetPr = Math.sqrt((double) (p * n) / m);
        int pr = 1;
        double minDiff = Double.MAX_VALUE;
        for (int d = 1; d <= p; d++) {
            if (p % d == 0 && d <= n) {
                double diff = Math.abs(d - targetPr);
                if (diff < minDiff) {
                    minDiff = diff;
                    pr = d;
                }
            }
        }
        int pc = p / pr;

        Thread[] threads = new Thread[p];
        int start = 0;
        for (int id = 0; id < p; id++) {
            threads[id] = new Thread(new BlockTask(this.F, this.C, this.V, id, pr, pc));
        }
        ConvolutionExecutor.startAndJoin(threads);
    }
}
