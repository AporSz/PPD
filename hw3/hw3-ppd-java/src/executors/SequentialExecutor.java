package executors;

import model.Matrix;
import tasks.HorizontalTask;
import tasks.SequentialTask;

public class SequentialExecutor implements ConvolutionExecutor{
    private Matrix F;
    private Matrix C;

    public SequentialExecutor(Matrix F, Matrix C) {
        this.F = F;
        this.C = C;
    }

    @Override
    public void executeTask(int p) throws InterruptedException {
        SequentialTask task = new SequentialTask(this.F, this.C);

        task.run();
    }
}
