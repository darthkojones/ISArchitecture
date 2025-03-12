package ex4;

import java.util.UUID;
import java.util.concurrent.ThreadLocalRandom;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;

public class Worker implements MqttCallback {

    private final String workerId;
    private final MqttClient client;
    private boolean exit = false;

    public Worker(String broker) throws MqttException {
        workerId = "dartThrower-" + UUID.randomUUID();
        client = new MqttClient(broker, workerId);
        client.setCallback(this);
        client.connect();
        client.subscribe("mqtt/coordinator/worker/" + workerId);
    }

    public void startWorking() throws MqttException, InterruptedException {
        requestTasks();

        while (!exit) {
            Thread.sleep(300);
        }

        client.disconnect();
        client.close();
        System.out.println("*mic drop* WORKER OUT!");
    }

    private void requestTasks() throws MqttException {
        MqttMessage request = new MqttMessage();
        client.publish("mqtt/coordinator/requests/" + workerId, request);
    }

    private long calculateHits(long darts) {
        int hits = 0;
        ThreadLocalRandom rng = ThreadLocalRandom.current();
  
        for(int i = 0; i < darts; i++) {
           double x = rng.nextDouble(-1.0, 1.0);
           double y = rng.nextDouble(-1.0, 1.0);
           if (x * x + y * y <= 1)  {
              hits++;
           }
        }
        return hits;
    }

    @Override
    public void messageArrived(String topic, MqttMessage message) throws Exception {
        long darts = Long.parseLong(message.toString());
        if (darts == 0) {
            exit = true;
            return;
        }

        long hits = calculateHits(darts);
        Thread.sleep(5000);  // Simulated delay

        String payload = darts + ":" + hits;
        client.publish("mqtt/coordinator/results/" + workerId, new MqttMessage(payload.getBytes()));
        requestTasks();
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
            Worker worker = new Worker("tcp://localhost:1883");
            worker.startWorking();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
