import model.Matrix;

void main() {
    Matrix matrix;
    try {
        matrix = Matrix.fromFile("test.txt");

        System.out.println(matrix);

        int n = matrix.getN();
        int m = matrix.getM();
        int[] arr = new int[n * m];

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                arr[i * m + j] = matrix.get(i, j);
            }
        }
        for (int i = 0; i < arr.length; i++) {
            System.out.print(arr[i] + " ");
        }

        matrix.toFile("out.txt");
    } catch (Exception e) {
        throw new RuntimeException(e);
    }
}