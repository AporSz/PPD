import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class Main {
    static List<Double> init_random(int n) {
        List<Double> list = new ArrayList<>();
        Random r = new Random();
        for (int i = 0; i < n; i++) {
            list.add(r.nextDouble());
        }

        return list;
    }

    static List<Double> init_zero(int n) {
        List<Double> list = new ArrayList<>();
        for (int i = 0; i < n; i++) {
            list.add(0.0);
        }

        return list;
    }

    static void add(List<Double> a, List<Double> b, List<Double> c) {
        for (int i = 0; i < a.size(); i++) {
            c.set(i, a.get(i) + b.get(i));
        }
    }

    public static void main(String[] args) {
        // 1. Check if exactly 2 arguments were provided
        if (args.length != 2) {
            System.out.println("Error: Please provide exactly two arguments.");
            System.out.println("Usage: java Main <argument1> <argument2>");
            return; // Exit the program early
        }

        String firstArg = args[0];
        String secondArg = args[1];

        int nr_of_threads = 0;
        int n = 0;
        try {
            n = Integer.getInteger(firstArg);
            nr_of_threads = Integer.getInteger(secondArg);
        }
        catch (NumberFormatException e) {
            System.out.println("Error: Please provide integers as arguments.");
        }

        List<Double> a = init_random(n);
        List<Double> b = init_random(n);
        List<Double> c = init_zero(n);

        Long begin = System.nanoTime();

        add(a, b, c);

        Long end = System.nanoTime();

        System.out.println(end - begin);
    }
}