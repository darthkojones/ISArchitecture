package com.iasex3;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class PiApproximation {
    private static final int TOTAL_DARTS = 1_000_000;
    // private static final int NUM_THREADS = 4; // hardcoded # of threads
    private static final int RT_THREADS = 8;//Runtime.getRuntime().availableProcessors(); // # of threads available to JVM
    // private static final int DARTS_PER_THREAD = 16; // this is based on NUM_THREADS
    private static final int DARTS_PER_THREAD_RT = 40; //trial and error shows that 40 DPT is the sweet spot in terms of performance //10 * RT_THREADS; // this is based on RT_THREADS
    private static final int NUM_IT = 10; // #iterations
    private static double AVG_TIME = 0; // keeps average execution time

    public static void main(String[] args) {
        System.out.println("RT_THREADS: " + RT_THREADS);
        System.out.println("DARTS_PER_THREAD_RT: " + DARTS_PER_THREAD_RT);
        System.out.println("running " + NUM_IT + " iterations...");

        double totalExecutionTime = 0.0;
        double totalPiValue = 0.0;

        for (int iter = 1; iter <= NUM_IT; iter++) {
            ExecutorService exec = Executors.newFixedThreadPool(RT_THREADS); // create thread pool RT to use all threads available to JVM, and NUM for a hardcoded number
            
            // split the darts into packets for each thread
            int dartsPerPackage = TOTAL_DARTS / (DARTS_PER_THREAD_RT * RT_THREADS);

            // start the timer (high precision)
            long startTime = System.nanoTime();
            int totalHits = 0;

            // so put all futures in an array
            List<Future<Integer>> futures = new ArrayList<>();

            // submit all tasks in parallel
            for (int i = 0; i < RT_THREADS * DARTS_PER_THREAD_RT; i++) {
                futures.add(exec.submit(new DartThrowingTask(dartsPerPackage)));
            }

            // collect results
            for (Future<Integer> future : futures) {
                try {
                    totalHits += future.get();
                    //System.out.println(" packg hits: " + totalHits);
                } catch (Exception e) {
                    // e.printStackTrace();
                    System.err.println("error - " + e);
                }
            }

            // stop the timer (high precision)
            long endTime = System.nanoTime();
            double piApprox = 4.0 * totalHits / TOTAL_DARTS;
            double timeTaken = (endTime - startTime) / 1e9; // Convert nanoseconds to seconds

            // Add to total execution time for averaging
            totalExecutionTime += timeTaken;
            totalPiValue += piApprox;

            System.out.println("iteration " + iter + " - Pi approx: " + piApprox + ", time: " + timeTaken + " seconds");

            // shutdown the thread pool
            exec.shutdown();
            try {
                if (!exec.awaitTermination(60, TimeUnit.SECONDS)) {
                    exec.shutdownNow();
                }
            } catch (InterruptedException e) {
                exec.shutdownNow();
            }
        }

        // average time
        AVG_TIME = totalExecutionTime / NUM_IT;
        double avgPi = totalPiValue / NUM_IT;

        System.out.println("\nfinal results after " + NUM_IT + " iterations:");
        System.out.println("RT_THREADS: " + RT_THREADS);
        System.out.println("DARTS_PER_THREAD_RT: " + DARTS_PER_THREAD_RT);
        System.out.println("AVG Pi: " + avgPi);
        System.out.println("AVG Time: " + AVG_TIME + " seconds");
    }
}
