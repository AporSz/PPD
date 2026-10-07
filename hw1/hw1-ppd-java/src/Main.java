import java.util.Random;

public class Main {
    static double[] init_random(int n) {
        double[] arr = new double[n];
        Random r = new Random();
        for (int i = 0; i < n; i++) {
            arr[i] = r.nextDouble() * 1000.0;
        }
        return arr;
    }

    static void add(double[] a, double[] b, double[] c) {
        for (int i = 0; i < a.length; i++) {
            c[i] = a[i] + b[i];
        }
    }

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Error: Please provide exactly two arguments.");
            return;
        }

        int n = 0;
        int nr_of_threads = 0;
        try {
            n = Integer.parseInt(args[0]);
            nr_of_threads = Integer.parseInt(args[1]);
        } catch (NumberFormatException e) {
            System.out.println("Error: Please provide integers as arguments.");
            return;
        }

        double[] a = init_random(n);
        double[] b = init_random(n);
        double[] c = new double[n];

        long begin = System.nanoTime();

        add(a, b, c);

        long end = System.nanoTime();

        System.out.println((end - begin) / 1_000_000.0);
    }
}