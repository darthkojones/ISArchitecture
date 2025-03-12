package ex4;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class Coordinator implements MqttCallback {

    private final MqttClient client;
    private final long TOTAL_DARTS = 69000;
    private final long DARTS_BATCH = 420;

    private long dartsLeft = TOTAL_DARTS;
    private long totalHits = 0;
    private long totalDartsThrown = 0;
    private boolean finished = false;

    public Coordinator(String broker) throws MqttException {
        client = new MqttClient(broker, "Coordinator");
        client.setCallback(this);
        client.connect();
        System.out.println("coordinator connected, let's fuckin' go!");

        client.subscribe("mqtt/coordinator/requests/+");
        client.subscribe("mqtt/coordinator/results/+");
        System.out.println("subscribed to request & result topics. bring it workers!");
    }

    private long allocateDarts() {
        if (dartsLeft >= DARTS_BATCH) {
            dartsLeft -= DARTS_BATCH;
            System.out.println("allocating " + DARTS_BATCH + " darts, " + dartsLeft + " left");
            return DARTS_BATCH;
        } else {
            long remaining = dartsLeft;
            dartsLeft = 0;
            System.out.println("last batch dudes! only " + remaining + " darts left");
            return remaining;
        }
    }

    public void runCoordinator() throws InterruptedException, MqttException {
        while (!finished) {
            Thread.sleep(300);
        }

        double piEstimate = 4.0 * (double) totalHits / totalDartsThrown;
        System.out.println("aaaaand we're done! estimated pi = " + piEstimate);

        Thread.sleep(3000);
        System.out.println("disconnecting and packing my shit up");
        client.disconnect();
        client.close();
        System.out.println("*peace out* coordinator out!");
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        String[] topicParts = topic.split("/");
        String workerId = topicParts[3];

        if (topic.contains("/requests/")) {
            System.out.println("worker [" + workerId + "] is beggin' for darts");
            long darts = allocateDarts();
            client.publish("mqtt/coordinator/worker/" + workerId, new MqttMessage(Long.toString(darts).getBytes()));
            System.out.println("sent " + darts + " darts to worker [" + workerId + "]");

        } else if (topic.contains("/results/")) {
            String[] results = message.toString().split(":");
            long dartsThrownNow = Long.parseLong(results[0]);
            long hitsNow = Long.parseLong(results[1]);

            totalDartsThrown += dartsThrownNow;
            totalHits += hitsNow;

            System.out.println("worker [" + workerId + "] threw " + dartsThrownNow + " darts and hit " + hitsNow + " bullseyes!");
            System.out.println("total darts thrown: " + totalDartsThrown + " | total hits: " + totalHits);

            if (totalDartsThrown >= TOTAL_DARTS) {
                finished = true;
                System.out.println("we reached " + TOTAL_DARTS + " darts, wrapping this shit up...");
                notifyWorkersToStop();
            }
        }
    }

    private void notifyWorkersToStop() throws MqttException {
        System.out.println("tellin' workers it's time to chill");
        client.publish("mqtt/coordinator/worker/all", new MqttMessage("0".getBytes()));
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.out.println("shit! we lost connection: " + cause.getMessage());
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // no biggie here, deliveries complete, chill
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
