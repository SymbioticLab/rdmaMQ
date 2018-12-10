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

import kafka.utils.ShutdownableThread;
import org.apache.kafka.clients.consumer.ConsumerConfig;
import org.apache.kafka.clients.consumer.ConsumerRecord;
import org.apache.kafka.clients.consumer.ConsumerRecords;
import org.apache.kafka.clients.consumer.KafkaConsumer;
import org.apache.kafka.common.TopicPartition;

import java.time.Duration;
import java.util.Collections;
import java.util.Properties;
import java.util.ArrayList;
import java.util.Collections;

public class Consumer extends ShutdownableThread {
    //private final KafkaConsumer<Integer, String> consumer;
    //private final KafkaConsumer<Integer, Integer> consumer;
    private final KafkaConsumer<Integer, byte[]> consumer;
    private final String topic;

    public Consumer(String topic) {
        super("KafkaConsumerExample", false);
        Properties props = new Properties();
        props.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, KafkaProperties.KAFKA_SERVER_URL + ":" + KafkaProperties.KAFKA_SERVER_PORT);
        props.put(ConsumerConfig.GROUP_ID_CONFIG, "DemoConsumer");
        props.put(ConsumerConfig.ENABLE_AUTO_COMMIT_CONFIG, "true");
        props.put(ConsumerConfig.AUTO_COMMIT_INTERVAL_MS_CONFIG, "1000");
        props.put(ConsumerConfig.SESSION_TIMEOUT_MS_CONFIG, "30000");
        /* the below three configs don't really limit the *real* batch size for consumer; can't really measure consumer single message latency */
        //props.put(ConsumerConfig.MAX_PARTITION_FETCH_BYTES_CONFIG, "4");    // Comment out when measure throughput!! Change value based on mesg (value) size
        //props.put(ConsumerConfig.FETCH_MAX_BYTES_CONFIG, "4");              // Comment out when measure throughput!! Change value based on mesg (value) size
        //props.put(ConsumerConfig.MAX_POLL_RECORDS_CONFIG, "500");             // Comment out when measure throughput!!
        props.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.IntegerDeserializer");
        //props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.StringDeserializer");
        //props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.IntegerDeserializer");
        props.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, "org.apache.kafka.common.serialization.ByteArrayDeserializer");

        consumer = new KafkaConsumer<>(props);
        this.topic = topic;
    }

    @Override
    public void doWork() {
        System.out.println("ENTER DO WORK");
        int numMessages = 0;
        //ArrayList<Long> lat = new ArrayList<Long>();
        long initTime = System.nanoTime();

        consumer.subscribe(Collections.singletonList(this.topic));

        /*
        // get the relatively recent offset
        consumer.seekToEnd(consumer.assignment());
        // need to poll once to make seekToEnd take effect
        consumer.poll(Duration.ofSeconds(1)); 
        System.out.println("PUPUPU");
        // set offset to curr pos - x
        
        System.out.println("partition set size = " + consumer.assignment().size());
        for (TopicPartition partition: consumer.assignment()) {
            long current_last_offset = consumer.position(partition);
            System.out.println("current_last_offset = " + current_last_offset);
            //consumer.seek(partition, current_last_offset - 1000000);       // watch out for offset
        }
        */ 

        while (true) {

            //long startTime = System.nanoTime();
            //ConsumerRecords<Integer, String> records = consumer.poll(Duration.ofSeconds(1));
            ////ConsumerRecords<Integer, Integer> records = consumer.poll(Duration.ofSeconds(5));
            //System.out.println("BEFORE POLL");
            ConsumerRecords<Integer, byte[]> records = consumer.poll(Duration.ofSeconds(1));
            //System.out.println("AFTER POLL");
            //long elapsedTime = System.nanoTime() - startTime;
            //lat.add(elapsedTime);
            //for (ConsumerRecord<Integer, String> record : records) {
            ////for (ConsumerRecord<Integer, Integer> record : records) {
            for (ConsumerRecord<Integer, byte[]> record : records) {
                //System.out.println("Received message: (" + record.key() + ", " + java.util.Arrays.toString(record.value()) + ") at offset " + record.offset());
                //System.out.println("Received message: (" + record.key() + ", " + record.value() + ") at offset " + record.offset());
                numMessages++;
            }
            //System.out.println("numMessages: " + numMessages);
            //if (numMessages > 1000) {      // smaller iteration used to measure LATENCY; Don't measure
            if (numMessages > 3000000) {     // larger iteration used to measure THROUGHPUT
                break;
            }
        }
        // Assume each poll gets only 1 message (actually most time it is the case)
        //System.out.println("PUPUPUPUPU " + numMessages);
        long totalTime = System.nanoTime() - initTime;
        //Collections.sort(lat);
        //int NUM_REQ = lat.size();   // calculating lat with batch
        //int idx_m = (int)Math.ceil(NUM_REQ * 0.5);
        //int idx_99 = (int)Math.ceil(NUM_REQ * 0.99);
        System.out.println("@Consumer MEASUREMENT:");
        //System.out.println("DEBUG lat = " + lat);
        System.out.println("Consumer num messages = " + numMessages);
        //System.out.println("Consumer NUM_REQ = " + NUM_REQ);
        //System.out.println("Consumer MEDIAN = " + (double)lat.get(idx_m)/1000 + " us");
        //System.out.println("Consumer 99 TAIL = " + (double)lat.get(idx_99)/1000 + " us");
        System.out.println("Consumer Throughput = " + (double)numMessages/(totalTime/(double)1000000000) + " mesg/sec");
        System.exit(1);
    }

    @Override
    public String name() {
        return null;
    }

    @Override
    public boolean isInterruptible() {
        return false;
    }
}
