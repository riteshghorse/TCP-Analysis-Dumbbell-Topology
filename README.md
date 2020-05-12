# TCP-Analysis-Dumbbell-Topology

This project aims at understanding of the behaviors of two different TCP protocols when congestion occurs in network. These two TCP protocols are Cubic and DCTCP. 

In this project, we assess the performance of TCP Cubic and DCTCP in terms of throughput and average flow completion time. Implementation of a dumbbell topology in the ns-3 simulator and ran
multiple experiments to evaluate the performance of both TCP versions for the given metrics. 

Once the topology is set and configurations are done, run the following experiments and measure
throughput and average flow completion time. In each experiment, use bulk sender application in ns-3 to
generate the traffic. Use bulk sender to send 50 GB of data from source to destination in each of the
following experiments:

Exp-1: S1 sends traffic to D1 using TCP Cubic. S2 and D2 are not used in this experiment.

Exp-2: S1 sends traffic to D1 and S2 sends traffic to D2. Both senders will use TCP Cubic and start
sending data to respective destinations simultaneously.

Exp-3: S1 sends traffic to D1 using DCTCP. S2 and D2 are not used in this experiment.

Exp-4: S1 sends traffic to D1 and S2 sends traffic to D2. Both senders will use DCTCP and start
sending data to respective destinations simultaneously.

Exp-5: S1 sends traffic to D1 using TCP Cubic whereas S2 sends traffic to D2 using DCTCP. Both
senders will start sending data to respective destinations simultaneously.
