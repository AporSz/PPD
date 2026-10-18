package executors;

import model.Matrix;
import tasks.HorizontalTask;
import tasks.SequentialTask;

public class SequentialExecutor implements ConvolutionExecutor{
    private Matrix F;
    private Matrix C;
    private Matrix V;

    public SequentialExecutor(Matrix F, Matrix C, Matrix V) {
        this.F = F;
        this.C = C;
        this.V = V;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        SequentialTask task = new SequentialTask(this.F, this.C, this.V);

        task.run();
    }
}
