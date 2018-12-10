/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package kafka.examples;

import org.apache.kafka.clients.producer.Callback;
import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.apache.kafka.clients.producer.ProducerConfig;
import org.apache.kafka.common.serialization.IntegerSerializer;
import org.apache.kafka.common.serialization.StringSerializer;
import org.apache.kafka.common.serialization.ByteArraySerializer;

import java.util.Properties;
import java.util.concurrent.ExecutionException;
import java.lang.Math;
import java.util.Arrays;

public class Producer extends Thread {
    //private final KafkaProducer<Integer, String> producer;
    ////private final KafkaProducer<Integer, Integer> producer;
    private final KafkaProducer<Integer, byte[]> producer;
    private final String topic;
    private final Boolean isAsync;

    public Producer(String topic, Boolean isAsync) {
        Properties props = new Properties();
        props.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, KafkaProperties.KAFKA_SERVER_URL + ":" + KafkaProperties.KAFKA_SERVER_PORT);
        props.put(ProducerConfig.CLIENT_ID_CONFIG, "DemoProducer");
        //props.put(ProducerConfig.BATCH_SIZE_CONFIG, "256");   // Comment out when measure throughput!! Change value based on mesg (value) size
        props.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, IntegerSerializer.class.getName());
        //props.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, StringSerializer.class.getName());
        //props.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, IntegerSerializer.class.getName());
        props.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, ByteArraySerializer.class.getName());
        producer = new KafkaProducer<>(props);
        this.topic = topic;
        this.isAsync = isAsync;
    }

    public void run() {
        int messageNo = 1;
        int NUM_REQ = 1000000;     // large num for measure THROUGHPUT; remember to use "async" mode
        //int NUM_REQ = 50000;     // small num for measure LATENCY; also remember to use "sync" mode
        long lat[] = new long[NUM_REQ];
        long initTime = System.currentTimeMillis();
        //int message = 233;
        byte[] message = new byte[256];
        Arrays.fill(message, (byte)5);
        ////int messageInt = 233;
        ////byte[] messageArr = new byte[4];
        //byte[] messageArr = "PUPUPU".getBytes();
        while (messageNo - 1 < NUM_REQ) {
        //while (true) {
            //String messageStr = "Message_" + messageNo;
            if (isAsync) { // Send asynchronously
                //long startTime = System.currentTimeMillis();
                producer.send(new ProducerRecord<>(topic,
                    messageNo,
                    message));
                    //messageArr));
                    //messageInt));
                    //messageInt), new DemoCallBack(startTime, messageNo, messageInt));
            } else { // Send synchronously
                try {
                    long startTime = System.nanoTime();
                    producer.send(new ProducerRecord<>(topic,
                        messageNo,
                        message)).get();
                        //messageArr)).get();
                        //messageInt)).get();
                    //System.out.println("Sent message: (" + messageNo + ", " + messageStr + ")");
                    long elapsedTime = System.nanoTime() - startTime;
                    //System.out.println("elapsedTime: " + elapsedTime);
                    lat[messageNo-1] = elapsedTime;
                } catch (InterruptedException | ExecutionException e) {
                    e.printStackTrace();
                }
            }
            ++messageNo;
        }
        System.out.println("@Producer MEASUREMENT:");
        if (!isAsync) {
            Arrays.sort(lat);
            int idx_m = (int)Math.ceil(NUM_REQ * 0.5);
            int idx_99 = (int)Math.ceil(NUM_REQ * 0.99);
            int idx_99_9 = (int)Math.ceil(NUM_REQ * 0.999);
            int idx_99_99 = (int)Math.ceil(NUM_REQ * 0.9999);
            System.out.println("Producer MEDIAN = " + (double)lat[idx_m]/(double)1000 + " us");
            System.out.println("Producer 99 TAIL = " + (double)lat[idx_99]/(double)1000 + " us");
            //System.out.println("Producer 99.9 TAIL = " + (double)lat[idx_99_9]/(double)1000 + " us");
            //System.out.println("Producer 99.99 TAIL = " + (double)lat[idx_99_99]/(double)1000 + " us");
        }
        long totalTime = System.currentTimeMillis() - initTime;
        System.out.println("Producer totalTime = " + (double)(totalTime) + " msec");
        System.out.println("Producer num messages = " + NUM_REQ);
        System.out.println("Producer messageNo = " + messageNo);
        //System.out.println("Producer totalTime = " + (double)(totalTime/1000000000) + " sec");
        System.out.println("Producer Throughput = " + (double)messageNo/(totalTime/(double)1000) + " mesg/sec");

    }
}

class DemoCallBack implements Callback {

    private final long startTime;
    private final int key;
    //private final String message;
    //private final int message;
    private final byte[] message;

    //public DemoCallBack(long startTime, int key, int message) {
    public DemoCallBack(long startTime, int key, byte[] message) {
        this.startTime = startTime;
        this.key = key;
        this.message = message;
    }

    /**
     * A callback method the user can implement to provide asynchronous handling of request completion. This method will
     * be called when the record sent to the server has been acknowledged. When exception is not null in the callback,
     * metadata will contain the special -1 value for all fields except for topicPartition, which will be valid.
     *
     * @param metadata  The metadata for the record that was sent (i.e. the partition and offset). Null if an error
     *                  occurred.
     * @param exception The exception thrown during processing of this record. Null if no error occurred.
     */
    public void onCompletion(RecordMetadata metadata, Exception exception) {
        long elapsedTime = System.currentTimeMillis() - startTime;
        //System.out.println("elapsedTime: " + elapsedTime);
        if (metadata != null) {
            System.out.println(
                "message(" + key + ", " + message + ") sent to partition(" + metadata.partition() +
                    "), " +
                    "offset(" + metadata.offset() + ") in " + elapsedTime + " ms");
        } else {
            exception.printStackTrace();
        }
    }
}
