package tasks;

import model.Matrix;

public class BlockTask implements Runnable {
    private Matrix F;
    private Matrix C;
    private Matrix V;
    private int id;
    private int pc;
    private int pr;

    public BlockTask(Matrix F, Matrix C, Matrix V, int id, int pr, int pc) {
        this.F = F;
        this.C = C;
        this.V = V;
        this.id = id;
        this.pr = pr;
        this.pc = pc;
    }

    @Override
    public void run() {
        int n = this.F.getN();
        int m = this.F.getM();
        int baseHeight = n / pr;
        int rowRemainder = n % pr;
        int baseWidth = m / pc;
        int columnRemainder = m % pc;
        int tr = id / pc;
        int tc = id % pc;

        int startRow = tr * baseHeight + Math.min(tr, rowRemainder);
        int endRow = startRow + baseHeight + (tr < rowRemainder ? 1 : 0);

        int startCol = tc * baseWidth + Math.min(tc, columnRemainder);
        int endCol = startCol + baseWidth + (tc < columnRemainder ? 1 : 0);

        for (int i = startRow; i < endRow; i++) {
            for (int j = startCol; j < endCol; j++) {
                int sum = Matrix.convolveAt(this.F, this.C, i, j);
                V.set(sum, i, j);
            }
        }
    }
}
