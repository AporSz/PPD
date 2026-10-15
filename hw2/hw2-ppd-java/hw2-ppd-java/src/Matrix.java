import java.util.List;

public class Matrix {
    private int[][] a;
    int n, m;

    public Matrix(int n, int m) {
        this.n = n;
        this.m = m;
        a = new int[n][m];
    }

    public int get(int i, int j) {
        if (i < 0) {
            if (j < 0) return a[0][0];

            return a[0][j];
        }
        if (j < 0) {
            return a[i][0];
        }

        if (i >= this.n) {
            if (j >= this.m) return a[n-1][m-1];

            return a[n-1][j];
        }
        if (j >= this.m) {
            return a[i][m-1];
        }

        return a[i][j];
    }

    
}
