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
//import org.apache.commons.cli.*;

public class KafkaConsumerProducerDemo {
    public static void main(String[] args) {
        //boolean isAsync = args.length == 0 || !args[0].trim().equalsIgnoreCase("sync");

        /*
        Options options = new Options();
        Option runAsync = new Option("a", "async", false, "Run asynchrously. Default is sync.");
        runAsync.setRequired(false);
        options.addOption(runAsync);
        Option role = new Option("r", "role", true, "\"producer\" or \"consumer\"");
        role.setRequired(true);
        options.addOption(role);

        CommandLineParser parser = new DefaultParser();
        HelpFormatter formatter = new HelpFormatter();
        CommandLine cmd = null;
        try {
            cmd = parser.parse(options, args);
        } catch(ParseException e) {
            System.err.println("Parsing failed.  Reason: " + e.getMessage());
            formatter.printHelp("microbenchmark", options);
            System.exit(1);
        }

        boolean isAsync = false;
        if (cmd.hasOption("async")) {
            isAsync = true;
            System.out.println("Running Asynchronously.");
        }
        if (cmd.getOptionValue("role") == "producer") {
            Producer producerThread = new Producer(KafkaProperties.TOPIC, isAsync);
            producerThread.start();
        } else if (cmd.getOptionValue("role") == "consumer") {
            Consumer consumerThread = new Consumer(KafkaProperties.TOPIC);
            consumerThread.start();
        } else {
            formatter.printHelp("microbenchmark", options);
            System.exit(1);
        }
        */

        String usage = "./xxx <async/sync> <producer/consumer>";
        if (args.length != 2) {
            System.out.println(usage);
            System.exit(1);
        }

        boolean isAsync = false;
        if (args[0].equals("async")) {
            isAsync = true;
        } else if (args[0].equals("sync")) {
            isAsync = false;
        } else {
            System.out.println(usage);
            System.exit(1);
        }

        boolean isProducer = true;
        if (args[1].equals("producer")) {
            isProducer = true;
        } else if (args[1].equals("consumer")) {
            isProducer = false;
        } else {
            System.out.println(usage);
            System.exit(1);
        }

        if (isProducer) {
            Producer producerThread = new Producer(KafkaProperties.TOPIC, isAsync);
            producerThread.start();
        } else {
            Consumer consumerThread = new Consumer(KafkaProperties.TOPIC);
            consumerThread.start();
        }

    }
}
