import java.io.*;
import java.nio.file.*;
import java.util.StringTokenizer;

public class Main {
    static boolean loadInputFile(String filename, int n, double[] a, double[] b) {
        File file = new File(filename);
        if (!file.exists()) return false;
        try (BufferedReader br = Files.newBufferedReader(file.toPath())) {
            String line = br.readLine();
            if (line == null) return false;
            int fileN = Integer.parseInt(line.trim());
            if (fileN != n) return false;

            String lineA = br.readLine();
            if (lineA == null) return false;
            StringTokenizer stA = new StringTokenizer(lineA);
            for (int i = 0; i < n; i++) {
                if (!stA.hasMoreTokens()) return false;
                a[i] = Double.parseDouble(stA.nextToken());
            }

            String lineB = br.readLine();
            if (lineB == null) return false;
            StringTokenizer stB = new StringTokenizer(lineB);
            for (int i = 0; i < n; i++) {
                if (!stB.hasMoreTokens()) return false;
                b[i] = Double.parseDouble(stB.nextToken());
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    static void writeOutputFile(String filename, double[] c) throws IOException {
        try (BufferedWriter bw = Files.newBufferedWriter(Paths.get(filename))) {
            bw.write(c.length + "\n");
            for (int i = 0; i < c.length; i++) {
                bw.write(String.format(java.util.Locale.US, "%.6f", c[i]) + (i == c.length - 1 ? "" : " "));
            }
            bw.write("\n");
        }
    }

    static boolean checkFilesEqual(String file1, String file2) {
        File f1 = new File(file1);
        File f2 = new File(file2);
        if (!f1.exists() || !f2.exists()) return false;
        try (BufferedReader br1 = Files.newBufferedReader(f1.toPath());
             BufferedReader br2 = Files.newBufferedReader(f2.toPath())) {
            String line1 = br1.readLine();
            String line2 = br2.readLine();
            if (line1 == null || line2 == null) return false;
            int n1 = Integer.parseInt(line1.trim());
            int n2 = Integer.parseInt(line2.trim());
            if (n1 != n2) return false;

            String lineA = br1.readLine();
            String lineB = br2.readLine();
            if (lineA == null || lineB == null) return false;
            StringTokenizer st1 = new StringTokenizer(lineA);
            StringTokenizer st2 = new StringTokenizer(lineB);

            for (int i = 0; i < n1; i++) {
                if (!st1.hasMoreTokens() || !st2.hasMoreTokens()) return false;
                double v1 = Double.parseDouble(st1.nextToken());
                double v2 = Double.parseDouble(st2.nextToken());
                if (Math.abs(v1 - v2) > 1e-4) return false;
            }
            return true;
        } catch (Exception e) {
            return false;
        }
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

        String inputFilename = "input.txt";
        String outputFilename = "output.txt";
        String seqOutputFilename = "output_seq.txt";

        double[] a = new double[n];
        double[] b = new double[n];

        try {
            if (!loadInputFile(inputFilename, n, a, b)) {
                System.err.println("Error: Could not load input file '" + inputFilename + "' with size " + n + ".");
                return;
            }

            double[] c = new double[n];

            long begin = System.nanoTime();

            add(a, b, c);

            long end = System.nanoTime();

            writeOutputFile(outputFilename, c);

            File seqFile = new File(seqOutputFilename);
            if (!seqFile.exists()) {
                writeOutputFile(seqOutputFilename, c);
            } else {
                if (!checkFilesEqual(outputFilename, seqOutputFilename)) {
                    System.err.println("Verification failed: Output file does not match sequential reference file.");
                }
            }

            System.out.println((end - begin) / 1_000_000.0);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}