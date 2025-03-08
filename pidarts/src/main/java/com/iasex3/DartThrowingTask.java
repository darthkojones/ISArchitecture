package com.iasex3;

import java.util.concurrent.Callable;
import java.util.concurrent.ThreadLocalRandom;
public class DartThrowingTask implements Callable<Integer> {
    private final int darts;
    public DartThrowingTask(int darts) {
        this.darts = darts;
    }
    @Override
    public Integer call() {
        int hits = 0;
        ThreadLocalRandom rng = ThreadLocalRandom.current();

        for (int i = 0; i < darts; i++) {
            double x = rng.nextDouble(-1,1); 
            double y = rng.nextDouble(-1,1); 



            // if (x * x + y * y <= 1) {
            //     hits++;
            // }

            
            if (Math.sqrt(x * x + y * y) <= 1) { //we can also calc the hypotenuse using Math.hypot(x, y)
                hits++;
            }
        }
        return hits;
    }


}
