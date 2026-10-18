package model;

import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.StringTokenizer;

public class Matrix {
    private int[][] a;
    int n, m;

    public Matrix(int n, int m) {
        this.n = n;
        this.m = m;
        this.a = new int[n][m];
    }

    public Matrix(int[][] a) {
        this.n = a.length;
        this.m = a[0].length;
        this.a = a;
    }

    public int get(int i, int j) {
        if (i < 0) {
            if (j < 0) return a[0][0];
            if (j >= this.m) return a[0][this.m-1];

            return a[0][j];
        }
        if (j < 0) {
            if (i >= this.n) return a[this.n-1][0];
            return a[i][0];
        }

        if (i >= this.n) {
            if (j >= this.m) return a[this.n-1][this.m-1];

            return a[this.n-1][j];
        }
        if (j >= this.m) {
            return a[i][this.m-1];
        }

        return a[i][j];
    }

    public void set(int v, int i, int j) {
        if (i < 0 || j < 0 || i >= this.n || j >= this.m) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        a[i][j] = v;
    }

    public int getN() {
        return this.n;
    }

    public int getM() {
        return this.m;
    }

    public static Matrix fromFile(String path) throws Exception {
        File file = new File(path);
        if (!file.exists()) throw new FileNotFoundException("File doesn't exist");
        try (BufferedReader br = Files.newBufferedReader(file.toPath())) {
            String line = br.readLine();
            if (line == null) throw new Exception("File empty");
            int n = Integer.parseInt(line.trim());
            line = br.readLine();
            if (line == null) throw new Exception("Invalid file");
            int m = Integer.parseInt(line.trim());

            int[][] a = new int[n][m];

            for (int i = 0; i < n; i++) {
                String lineA = br.readLine();
                if (lineA == null) throw new Exception("Invalid file");
                StringTokenizer stA = new StringTokenizer(lineA);
                for (int j = 0; j < m; j++) {
                    if (!stA.hasMoreTokens()) throw new Exception("Invalid file");
                    a[i][j] = Integer.parseInt(stA.nextToken());
                }
            }

            return new Matrix(a);

        } catch (Exception e) {
            throw new Exception(e.getMessage());
        }
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();

        for (int i = 0; i < this.n; i++) {
            for (int j = 0; j < this.m; j++) {
                sb.append(a[i][j]);
                sb.append(" ");
            }
            sb.append("\n");
        }

        return sb.toString();
    }

    public void toFile(String path) {
        try (BufferedWriter bw = Files.newBufferedWriter(Paths.get(path))) {
            bw.write(this.n + "\n");
            bw.write(this.m + "\n");
            for (int i = 0; i < this.n; i++) {
                for (int j = 0; j < this.m; j++) {
                    bw.write(a[i][j] + " ");
                }
                bw.write("\n");
            }

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static int convolveAt(Matrix F, Matrix C, int i, int j) {
        int sum = 0;
        int half_size = C.getN() / 2;

        for (int k = -half_size; k <= half_size; k++) {
            for (int l = -half_size; l <= half_size; l++) {
                sum += F.get(i + k, j + l) * C.get(k + half_size, l + half_size);
            }
        }

        return sum;
    }

    public void reAllocate() {
        this.a = new int[this.n][this.m];
    }

    public boolean equals(Matrix other) {
        if (other == this) return true;

        if (this.n != other.getN() || this.m != other.getM()) return false;

        for (int i = 0; i < this.n; i++) {
            for (int j = 0; j < this.m; j++) {
                if (this.a[i][j] != other.get(i, j)) {
                    return false;
                }
            }
        }

        return true;
    }
}