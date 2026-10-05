import java.util.Random;

public class Main {
    public static void main(String[] args) throws InterruptedException {
        int n = 100000000;
        int threads = 10;
        double[] a = new double[n];
        double[] b = new double[n];
        Random random = new Random();
        for(int i =0; i < n; i++){
            a[i] = random.nextDouble();
            b[i] = random.nextDouble();
        }

        double[] c = new double[n];
        double[] d = new double[n];

        long start = System.nanoTime();
        add(a, b, c);
        long end = System.nanoTime();
        System.out.println(end - start);

        start = System.nanoTime();
        threaded_add(a, b, d, threads);
        end = System.nanoTime();
        System.out.println(end - start);
    }

    static void add(double[] a, double[] b, double[] c) {
        for(int i = 0; i < a.length; i++){
            c[i] = a[i] + b[i];
        }
    }

    //functie de doua cerculete
    static void threaded_add(double[] a, double[] b, double[] c, int nr_of_threads) throws InterruptedException {
        Thread[] threads = new Thread[nr_of_threads];

        int rest = a.length % nr_of_threads;

        for (int i = 0; i < nr_of_threads; i++) {
            int start = i * a.length / nr_of_threads + Math.min(i, rest);
            int end = start + a.length / nr_of_threads + ((i < rest) ? 1 : 0);

            threads[i] = new Thread(
                    () -> {
                        for (int j = start; j < end; j++) {
                            c[j] = a[j] + b[j];
                        }
                    }
            );

            threads[i].start();
        }

        for (int i = 0; i < nr_of_threads; i++) {
            threads[i].join();
        }
    }
}