package ex4;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class Coordinator implements MqttCallback {

    private final MqttClient client;
    private final long TOTAL_DARTS = 100000;
    private final long DARTS_BATCH = 1000;

    private long dartsLeft = TOTAL_DARTS;
    private long totalHits = 0;
    private long totalDartsThrown = 0;
    private boolean finished = false;

    public Coordinator(String broker) throws MqttException {
        client = new MqttClient(broker, "Coordinator");
        client.setCallback(this);
        client.connect();
        client.subscribe("mqtt/coordinator/requests/+");
        client.subscribe("mqtt/coordinator/results/+");
    }

    private long allocateDarts() {
        if (dartsLeft >= DARTS_BATCH) {
            dartsLeft -= DARTS_BATCH;
            return DARTS_BATCH;
        } else {
            long remaining = dartsLeft;
            dartsLeft = 0;
            return remaining;
        }
    }

    public void runCoordinator() throws InterruptedException, MqttException {
        while (!finished) {
            Thread.sleep(300);
        }

        double piEstimate = 4.0 * (double) totalHits / totalDartsThrown;
        System.out.println("Estimated PI: " + piEstimate);

        Thread.sleep(3000);
        client.disconnect();
        client.close();
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        String[] topicParts = topic.split("/");
        String workerId = topicParts[3];

        if (topic.contains("/requests/")) {
            long darts = allocateDarts();
            client.publish("mqtt/coordinator/worker/" + workerId, new MqttMessage(Long.toString(darts).getBytes()));
        } else if (topic.contains("/results/")) {
            String[] results = message.toString().split(":");
            totalDartsThrown += Long.parseLong(results[0]);
            totalHits += Long.parseLong(results[1]);
            if (totalDartsThrown >= TOTAL_DARTS) finished = true;
        }
    }

    @Override
    public void connectionLost(Throwable cause) {
        System.out.println("Connection lost: " + cause.getMessage());
    }

    @Override
    public void deliveryComplete(IMqttDeliveryToken token) {
        // delivery confirmation (optional logging)
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
