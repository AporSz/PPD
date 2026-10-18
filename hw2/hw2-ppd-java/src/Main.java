import executors.*;
import model.Matrix;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.Locale;

Matrix F;
Matrix C;
Matrix V;
int p;

ConvolutionExecutor getExecutorType(String type) throws Exception {
    if (type.toLowerCase(Locale.ROOT).equals("s")) return new SequentialExecutor(F, C, V);
    if (type.toLowerCase(Locale.ROOT).equals("h")) return new HorizontalExecutor(F, C, V);
    if (type.toLowerCase(Locale.ROOT).equals("v")) return new VerticalExecutor(F, C, V);
    if (type.toLowerCase(Locale.ROOT).equals("b")) return new BlockExecutor(F, C, V);
    if (type.toLowerCase(Locale.ROOT).equals("l")) return new LinearExecutor(F, C, V);
    if (type.toLowerCase(Locale.ROOT).equals("c")) return new CyclicExecutor(F, C, V);

    throw new Exception("Invalid executor type");
}

void timeAndExecute(ConvolutionExecutor executor, SequentialExecutor seq, String outputPath, String sequentialPath) {
    File seqFile = new File(sequentialPath);
    if (!seqFile.exists() && executor.getClass() != seq.getClass()) {
        try {
            seq.executeTask(1);
            V.toFile(sequentialPath);
            V.reAllocate();
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    long t0 = System.nanoTime();

    try {
        executor.executeTask(p);
    } catch (InterruptedException e) {
        throw new RuntimeException(e);
    }

    long t1 = System.nanoTime();

    V.toFile(outputPath);

    if (executor.getClass() == seq.getClass()) {
        if (!seqFile.exists()) {
            V.toFile(sequentialPath);
        }
    } else {
        if (!verify(sequentialPath, outputPath)) {
            throw new RuntimeException("Incorrect solution");
        }
    }

    double ms = (t1 - t0) / 1000000.0;
    System.out.println(ms);
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
        return false;
    }
}

void main(String[] args) {
    if (args.length != 2) {
        throw new RuntimeException("Error: Please provide exactly two arguments.");
    }

    String type;
    try {
        p = Integer.parseInt(args[0]);
        type = args[1];
        if (type.length() != 1) throw new Exception("Invalid executor type");
    } catch (NumberFormatException e) {
        throw new RuntimeException("Error: Please provide integers as arguments.");
    } catch (Exception e) {
        throw new RuntimeException(e);
    }

    String matrixF_path = "../files/MatrixF.txt";
    String convolutionM_path = "../files/ConvolutionM.txt";
    String sequentialPath = "../files/sequential.txt";
    String outputPath = "../files/output.txt";

    try {
        F = Matrix.fromFile(matrixF_path);
        C = Matrix.fromFile(convolutionM_path);
        V = new Matrix(F.getN(), F.getM());

        SequentialExecutor seq = new SequentialExecutor(F, C, V);
        ConvolutionExecutor executor = getExecutorType(type);

        timeAndExecute(executor, seq, outputPath, sequentialPath);

    } catch (Exception e) {
        throw new RuntimeException(e);
    }
}