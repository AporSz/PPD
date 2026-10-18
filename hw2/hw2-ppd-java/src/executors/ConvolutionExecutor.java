package executors;

public interface ConvolutionExecutor {
    public void executeTask(int p) throws InterruptedException;

    static void startAndJoin(Thread[] threads) throws InterruptedException {
        for (Thread t : threads) {
            t.start();
        }
        for (Thread t : threads) {
            t.join();
        }
    }
}