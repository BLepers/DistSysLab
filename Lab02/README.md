# TP 02 — From Reliable Broadcasts to Byzantine tolerance

## Goal
Implement various broadcast algorithms, and discuss their reliability and performance.


## Algorithm 1 - Raft Broadcast Algorithm

The Raft Broadcast Algorithm is a consensus algorithm designed to manage a replicated log. It ensures that all nodes in a distributed system agree on the same sequence of log entries, even in the presence of failures. For simplicity, we assume that a single leader has already been elected, and only that machine initiates broadcasts. To simplify the implementation, we also assume that the leader only broadcasts integer values.

### Pseudo code
1. **Propose:**  
   The leader sends `[Propose, msg]` to all followers.
2. **Receive Propose:**  
   Send `[Ack]` to the leader.
3. **Receive ACK:**  
   When a **majority** of followers have acknowledged the message, deliver the message and send `[Commit]` to all followers.
4. **Receive Commit:**  
   Deliver the message.

⚠️  In the actual Raft algorithm, the messages have an index and a term number, to ensure that they are inserted in the correct order in the log. Here, we ignore these details for simplicity.


## Algorithm 2 - Bachar Broadcast Algorithm
The Bachar Broadcast Algorithm is a reliable broadcast protocol that ensures that if a correct process broadcasts a message, then all correct processes will eventually deliver that message. It is designed to handle message loss and process failures in a distributed system.

### Pseudo code

In this algorithm, f = (n-1)/3, where n is the total number of processes.

1. **Propose:** 
   The leader sends `[Propose, msg]` to all followers.
2. **Receive Propose:**
   If msg has not been received before, send `[Echo, msg]` to all processes.
3. **Receive `[Echo,msg]` from n-f distinct processes:**
   Send `[Vote, msg]` to all processes.
4. **Receive `[Vote,msg]` from f+1 distinct processes:**
   Send `[Vote, msg]` to all processes.
5. **Receive `[Vote,msg]` from n-f distinct processes:**
   Deliver msg.
   
⚠️ It is important to compare the "msg" field when counting requests. [Echo,1] and [Echo,2] are different messages.

### Input

All processes are started as `./process <id> <filename>`, with filename following the format:
```
N # number of processes
BCAST 1 X # process 0 sends value "1" to X processes, X may be != N
BCAST 2 Y # optional, process 0 sends value "2" to Y processes, starting from process X+1, X+Y <= N
```

You can assume that a process only delivers a single message and then exits. The broadcasted value is always an integer.

### Output

When a process delivers a message, it prints:
```
[Process \d+] DELIVERED message # E.g.: [Process 3] DELIVERED 42
```

## Grading (Tentative)
- **Required (but not sufficient) to pass:** Both algorithms are implemented and deliver messages correctly in non-faulty cases.
- Write a report discussing the following:
  - **Performance**: Set N from N=4 to N=100 and measure the latency of both algorithms (time between Propose and Deliver on the leader). Plot the results and comment on the practicality of both algorithms. You can assume a non-faulty case in which the leader sends a single "BCAST 1 N" (i.e., a single broadcast to all processes).
  - **Fault tolerance**: Launch both algorithms with N=4, N=7, and N=10, and with various values of X in "BCAST 1 X". What is the minimum value of X to ensure that all non-faulty processes deliver the message?
  - **Byzantine tolerance**: Have the leader send different messages to different processes (i.e., "BCAST msg1 X; BCAST msg2 N-X"). Under which conditions do non-faulty process deliver different messages?
  - (Optional, but will give a significant bonus) Have malicious followers send different messages to different processes.
    - At step 2 some malicious followers collude and agree to send [Echo,"HACK"] instead of the correct message. How many malicious followers can you tolerate before non-malicious processes deliver different messages?
    - Have the leader send
