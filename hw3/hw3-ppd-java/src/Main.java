import executors.ConvolutionExecutor;
import executors.HorizontalExecutor;
import executors.SequentialExecutor;
import executors.VerticalExecutor;
import model.Matrix;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.Locale;

Matrix F;
Matrix C;

ConvolutionExecutor getExecutorType(String type) {
    if (type.equals("s")) return new SequentialExecutor(F, C);
    if (type.equals("h")) return new HorizontalExecutor(F, C);
    if (type.equals("v")) return new VerticalExecutor(F, C);
    throw new IllegalArgumentException("Executor must be 's', 'h', or 'v'");
}

boolean verify(String sequentialPath, String outputPath) {
    File fSeq = new File(sequentialPath);
    File fOut = new File(outputPath);
    if (!fSeq.exists() || !fOut.exists()) return false;

    try (BufferedReader brSeq = Files.newBufferedReader(fSeq.toPath());
         BufferedReader brOut = Files.newBufferedReader(fOut.toPath())) {
        String lineSeq;
        String lineOut;
        while ((lineSeq = brSeq.readLine()) != null) {
            lineOut = brOut.readLine();
            if (lineOut == null || !lineSeq.trim().equals(lineOut.trim())) {
                return false;
            }
        }
        return brOut.readLine() == null;
    } catch (IOException e) {
        throw new RuntimeException("Could not compare sequential and parallel outputs", e);
    }
}

void main(String[] args) {
    if (args.length != 2) {
        throw new IllegalArgumentException("Expected arguments <threads> <s|h|v>.");
    }

    final int threads;
    final String type = args[1].toLowerCase(Locale.ROOT);
    try {
        threads = Integer.parseInt(args[0]);
    } catch (NumberFormatException e) {
        throw new IllegalArgumentException("Thread count must be an integer.", e);
    }
    if (threads <= 0) throw new IllegalArgumentException("Thread count must be positive.");
    if (!type.equals("s") && !type.equals("h") && !type.equals("v")) {
        throw new IllegalArgumentException("Executor must be 's', 'h', or 'v'.");
    }

    String inputPath = "../files/date.txt";
    String convolutionPath = "../files/ConvolutionM.txt";
    String sequentialPath = "../files/sequential.txt";
    String outputPath = "../files/output.txt";

    try {
        F = Matrix.fromFile(inputPath);
        C = Matrix.fromFile(convolutionPath);
        if (C.getN() != 3 || C.getM() != 3) {
            throw new IllegalArgumentException("HW3 requires a 3x3 convolution matrix.");
        }
        if (type.equals("h") && threads > F.getN()) {
            throw new IllegalArgumentException("Line-based thread count cannot exceed the number of rows.");
        }
        if (type.equals("v") && threads > F.getM()) {
            throw new IllegalArgumentException("Line-based thread count cannot exceed the number of columns.");
        }

        ConvolutionExecutor executor = getExecutorType(type);
        long t0 = System.nanoTime();
        executor.executeTask(type.equals("s") ? 1 : threads);
        long t1 = System.nanoTime();

        F.toFile(outputPath);
        if (type.equals("s")) {
            F.toFile(sequentialPath);
        } else if (!verify(sequentialPath, outputPath)) {
            throw new IllegalStateException("Incorrect solution or missing sequential reference.");
        }

        System.out.println((t1 - t0) / 1_000_000.0);
    } catch (Exception e) {
        throw new RuntimeException(e);
    }
}
