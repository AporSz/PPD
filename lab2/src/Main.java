import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;

public class Main {

    // ---------- 1. Generate a vector with n random real values in [1, 100) ----------
    static double[] generateVector(int n) {
        Random random = new Random();
        double[] v = new double[n];
        for (int i = 0; i < n; i++) {
            v[i] = 1 + 99 * random.nextDouble(); // result[i]=random.nextInt(1000);
        }
        return v;
    }

    static List<Double> generateList(int n) {
        Random random = new Random();
        List<Double> v = initList(n);

        for (int i = 0; i < n; i++) {
            v.set(i, 1 + 99 * random.nextDouble()); // result[i]=random.nextInt(1000);
        }
        return v;
    }

    static List<Double> initList(int n) {
        List<Double> v = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            v.add(0.0);
        }

        return v;
    }

    // ---------- 2. The two operations on one pair of elements ----------
    static double add(double a, double b) {
        return a + b;
    }

    static double sqrtOfSquares(double a, double b) {
        return Math.sqrt(a * a * a * a + b * b * b * b);
    }

    static double complexOp(double a, double b) {
        return Math.pow(a, b);
    }

    static double complexOp2(double a, double b) {
        return (a % b + a) * b - 1;
    }

    // The operation used by ALL versions (sequential, linear, cyclic).
    // To switch the experiment, change only this line:
    //   return add(a, b);   or   return sqrtOfSquares(a, b);
    static double operation(double a, double b) {
        return complexOp2(a, b);
    }

    // ---------- 3. Print a vector ----------
    static void printVector(String name, double[] v) {
        System.out.println(name + " = " + Arrays.toString(v));
    }

    // ---------- 4. Sequential version ----------
    static void sequential(double[] a, double[] b, double[] c) {
        for (int i = 0; i < c.length; i++) {
            c[i] = operation(a[i], b[i]);
        }
    }

    static void sequentialList(List<Double> a, List<Double> b, List<Double> c) {
        for (int i = 0; i < c.size(); i++) {
            c.set(i, operation(a.get(i), b.get(i)));
        }
    }

    // ---------- 5. Linear (block) distribution ----------
    // Runnable = the "note" with the work to do. Each thread gets one segment: start .. end-1
    static class LinearTask implements Runnable {
        private final double[] a, b, c;
        private final int start, end;

        LinearTask(double[] a, double[] b, double[] c, int start, int end) {
            this.a = a;
            this.b = b;
            this.c = c;
            this.start = start;
            this.end = end;
        }

        @Override
        public void run() {
            for (int i = start; i < end; i++) {
                c[i] = operation(a[i], b[i]);
            }
        }
    }

    static class LinearListTask implements Runnable {
        private final List<Double> a, b, c;
        private final int start, end;

        LinearListTask(List<Double> a, List<Double> b, List<Double> c, int start, int end) {
            this.a = a;
            this.b = b;
            this.c = c;
            this.start = start;
            this.end = end;
        }

        @Override
        public void run() {
            for (int i = start; i < end; i++) {
                c.set(i, operation(a.get(i), b.get(i)));
            }
        }
    }

    static class CyclicTask implements Runnable {
        private final double[] a, b, c;
        private final int id, p;

        CyclicTask(double[] a, double[] b, double[] c, int id, int p) {
            this.a = a;
            this.b = b;
            this.c = c;
            this.id = id;
            this.p = p;
        }

        @Override
        public void run() {
            for (int i = id; i < a.length; i = i + p) {
                c[i] = operation(a[i], b[i]);
            }
        }
    }

    static class CyclicListTask implements Runnable {
        private final List<Double> a, b, c;
        private final int id, p;

        CyclicListTask(List<Double> a, List<Double> b, List<Double> c, int id, int p) {
            this.a = a;
            this.b = b;
            this.c = c;
            this.id = id;
            this.p = p;
        }

        @Override
        public void run() {
            for (int i = id; i < a.size(); i = i + p) {
                c.set(i, operation(a.get(i), b.get(i)));
            }
        }
    }

    static void linear(double[] a, double[] b, double[] c, int p) throws InterruptedException {
        int n = c.length;
        int size = n / p;    // every thread gets at least 'size' elements
        int extra = n % p;   // the first 'extra' threads get one more element

        Thread[] threads = new Thread[p];
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size;
            if (id < extra) {
                end++;
            }
            threads[id] = new Thread(new LinearTask(a, b, c, start, end));
            start = end;
        }
        startAndJoin(threads);
    }

    static void linearList(List<Double> a, List<Double> b, List<Double> c, int p) throws InterruptedException {
        int n = c.size();
        int size = n / p;    // every thread gets at least 'size' elements
        int extra = n % p;   // the first 'extra' threads get one more element

        Thread[] threads = new Thread[p];
        int start = 0;
        for (int id = 0; id < p; id++) {
            int end = start + size;
            if (id < extra) {
                end++;
            }
            threads[id] = new Thread(new LinearListTask(a, b, c, start, end));
            start = end;
        }
        startAndJoin(threads);
    }

    // ---------- 6. Cyclic distribution ----------
    // Thread 'id' processes the indices id, id + p, id + 2p, ...

    static void cyclic(double[] a, double[] b, double[] c, int p) throws InterruptedException {
        Thread[] threads = new Thread[p];

        for (int i=0; i<p; i++) {
            threads[i] = new Thread(new CyclicTask(a, b, c, i, p));
        }

        startAndJoin(threads);
    }

    static void cyclicList(List<Double> a, List<Double> b, List<Double> c, int p) throws InterruptedException {
        Thread[] threads = new Thread[p];

        for (int i=0; i<p; i++) {
            threads[i] = new Thread(new CyclicListTask(a, b, c, i, p));
        }

        startAndJoin(threads);
    }

    // ---------- Start ALL threads first, then wait for ALL of them ----------
    static void startAndJoin(Thread[] threads) throws InterruptedException {
        for (Thread t : threads) {
            // start() creates a NEW thread, and that new thread executes run().
            // If we called t.run() instead, no new thread would exist:
            // run() would be executed by the main thread, one task after another.
            t.start();
        }
        for (Thread t : threads) {
            // join() = "main waits here until thread t has finished".
            // Without it, main could use vector C while the threads are still writing in it.
            // Starting everything first and joining afterwards keeps the threads running together.
            t.join();
        }
    }

    // ---------- 7. Measure the three versions and print one line ----------
    static void measure(double[] a, double[] b, double[] c, int p) throws InterruptedException {
        double seqMs = 0, linMs = 0, cycMs = 0;

        for (int round = 0; round < 2; round++) {   // round 0 = warm-up, round 1 = kept
            long t0 = System.nanoTime();
            sequential(a, b, c);
            seqMs = (System.nanoTime() - t0) / 1e6;

            t0 = System.nanoTime();
            linear(a, b, c, p);
            linMs = (System.nanoTime() - t0) / 1e6;

            t0 = System.nanoTime();
            cyclic(a, b, c, p);
            cycMs = (System.nanoTime() - t0) / 1e6;
        }

        System.out.printf("n=%-9d p=%-3d sequential=%9.3f ms  linear=%9.3f ms  cyclic=%9.3f ms%n",
                a.length, p, seqMs, linMs, cycMs);
    }

    static void measureList(List<Double> a, List<Double> b, List<Double> c, int p) throws InterruptedException {
        double seqMs = 0, linMs = 0, cycMs = 0;

        for (int round = 0; round < 2; round++) {   // round 0 = warm-up, round 1 = kept
            long t0 = System.nanoTime();
            sequentialList(a, b, c);
            seqMs = (System.nanoTime() - t0) / 1e6;

            t0 = System.nanoTime();
            linearList(a, b, c, p);
            linMs = (System.nanoTime() - t0) / 1e6;

            t0 = System.nanoTime();
            cyclicList(a, b, c, p);
            cycMs = (System.nanoTime() - t0) / 1e6;
        }

        System.out.printf("n=%-9d p=%-3d sequential=%9.3f ms  linear=%9.3f ms  cyclic=%9.3f ms%n",
                a.size(), p, seqMs, linMs, cycMs);
    }

    // ---------- Check correctness on a small example: n = 10, p = 3 ----------
    static void verify() throws InterruptedException {
        int n = 10, p = 3;
        double[] a = generateVector(n);
        double[] b = generateVector(n);
        double[] expected = new double[n];
        double[] c = new double[n];

        sequential(a, b, expected);
        linear(a, b, c, p);
        boolean linearOk = Arrays.equals(expected, c);

        Arrays.fill(c, 0);
        cyclic(a,b,c,p);
        boolean cyclicOk = Arrays.equals(expected, c);


        printVector("A         ", a);
        printVector("B         ", b);
        printVector("sequential", expected);
        printVector("cyclic    ", c);
        System.out.println("linear correct: " + linearOk);
        System.out.println("cyclic correct: " + cyclicOk);
        System.out.println();
    }

    static void verifyList() throws InterruptedException {
        int n = 10, p = 3;
        List<Double> a = generateList(n);
        List<Double> b = generateList(n);
        List<Double> expected = initList(n);
        List<Double> c = initList(n);

        sequentialList(a, b, expected);
        linearList(a, b, c, p);
        boolean linearOk = expected.equals(c);

        c = initList(n);
        cyclicList(a,b,c,p);
        boolean cyclicOk = expected.equals(c);


        System.out.println("A         " + a);
        System.out.println("B         " + b);
        System.out.println("sequential" + expected);
        System.out.println("cyclic    " + c);
        System.out.println("linear correct: " + linearOk);
        System.out.println("cyclic correct: " + cyclicOk);
        System.out.println();
    }

    public static void main(String[] args) throws InterruptedException {
        System.out.println("Processors: " + Runtime.getRuntime().availableProcessors());
        verify();

        int[] sizes = {1000, 10000, 100000, 1000000, 10000000};
        int[] threadCounts = {4, 6, 8, 16};

        for (int n : sizes) {
            double[] a = generateVector(n);
            double[] b = generateVector(n);
            double[] c = new double[n];
            for (int p : threadCounts) {
                measure(a, b, c, p);
            }
            System.out.println();
        }

        System.out.println();
        System.out.println();
        System.out.println("Array List version");
        System.out.println();
        verifyList();

        for (int n : sizes) {
            List<Double> a = generateList(n);
            List<Double> b = generateList(n);
            List<Double> c = initList(n);

            for (int p : threadCounts) {
                measureList(a, b, c, p);
            }
            System.out.println();
        }
    }
}