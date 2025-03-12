package ex4;

import java.util.HashSet;
import java.util.Set;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class Coordinator implements MqttCallback {

    private final MqttClient client;
    private final long TOTAL_DARTS = 69_000;
    private final long DARTS_BATCH = 420;

    private long dartsLeft = TOTAL_DARTS;
    private long totalHits = 0;
    private long totalDartsThrown = 0;
    private boolean finished = false;
    private final Set<String> activeWorkers = new HashSet<>();

    public Coordinator(String broker) throws MqttException {
        client = new MqttClient(broker, "Coordinator");
        client.setCallback(this);
        client.connect();
        System.out.println("coordinator connected, let's fuckin' go!");
        System.out.println("total darts to throw today " + TOTAL_DARTS);

        client.subscribe("mqtt/coordinator/requests/+");
        client.subscribe("mqtt/coordinator/results/+");
        System.out.println("subscribed to request & result topics. bring it workers!");
    }

    private long allocateDarts() {
        if (dartsLeft >= DARTS_BATCH) {
            dartsLeft -= DARTS_BATCH;
            return DARTS_BATCH;
        } else {
            long remaining = dartsLeft;
            dartsLeft = 0;
            System.out.println("last batch dudes! only " + remaining + " darts left");
            return remaining;
        }
    }

    public void runCoordinator() throws InterruptedException, MqttException {
        long startTime = System.nanoTime();
        System.out.println("timer started, let's do this!");

        while (!finished) {
            Thread.sleep(300);
        }

        double piApproximation = 4.0 * (double) totalHits / totalDartsThrown;
        long endTime = System.nanoTime();
        double timeElapsed = (double) (endTime - startTime) / 1_000_000_000.0;
        System.out.println("estimated pi: " + piApproximation);
        System.out.println("finished in: " + timeElapsed + " seconds with " + activeWorkers.size() + " badass worker(s)!");

        Thread.sleep(3000);
        client.disconnect();
        client.close();
        System.out.println("*peace out* coordinator out!");
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        String[] topicParts = topic.split("/");
        String workerId = topicParts[3];

        if (topic.contains("/requests/")) {
            activeWorkers.add(workerId);
            long darts = allocateDarts();
            client.publish("mqtt/coordinator/worker/" + workerId, new MqttMessage(Long.toString(darts).getBytes()));
            System.out.println("sent " + darts + " darts to worker [" + workerId + "], we have "  + dartsLeft + " darts left to give out");

        } else if (topic.contains("/results/")) {
            String[] results = message.toString().split(":");
            long dartsThrownNow = Long.parseLong(results[0]);
            long hitsNow = Long.parseLong(results[1]);

            totalDartsThrown += dartsThrownNow;
            totalHits += hitsNow;

            System.out.println("worker [" + workerId + "] scored " + hitsNow + " hits from " + dartsThrownNow + " darts");

            if (totalDartsThrown >= TOTAL_DARTS) {
                finished = true;
                System.out.println("we're done here boys, all darts thrown!");
                notifyWorkersToStop();
            }
        }
    }

    private void notifyWorkersToStop() throws MqttException {
        MqttMessage stopMessage = new MqttMessage("0".getBytes());
        client.publish("mqtt/coordinator/worker/all", stopMessage);
        System.out.println("told everyone to chill, no more darts left");
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.out.println("connection lost: " + cause.getMessage());
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // not important for now
    }

    public static void main(String[] args) {
        try {
            Coordinator coordinator = new Coordinator("tcp://localhost:1883");
            coordinator.runCoordinator();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
