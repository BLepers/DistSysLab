# TP 01 — Distributed Lock with Lamport Clocks

## Goal
Create a **live** and **fair** distributed lock using Lamport Clocks.

## Lock Algorithm

### Synchronization Requirements
1. A process must release the resource before another can use it (**mutual exclusion**).  
2. Requests must be granted in the order they are made (**fairness**).  
3. If all processes eventually release the resource, then all requests are eventually granted (**liveness**).  

### Total order using Lamport Clocks
Lamport Clocks alone are not sufficient to order events globally because two processes might have the same local clock at the same time. We extend Lamport Clocks to define a total ordering (⇒). If `a` is an event in process `Pi` and `b` in process `Pj`, then `a ⇒ b` iff:
- (i) `LCi(a) < LCj(b)`  
- (ii) `LCi(a) = LCj(b)` and `Pi < Pj`

### Pseudo-code
Each process has a queue totally ordered by LC and process ID (total order).

1. **Request:**  
   Process `Pi` sends `[Req, LCi, Pi]` to all processes and adds it to its queue.  
2. **Receive Request:**  
   Process `Pj` adds `[Req, LCi, Pi]` to its queue and sends `[Ack, LCj, Pj]` to `Pi`.  
3. **Release:**  
   Process `Pi` removes `[Req, LC, Pi]` from its queue and broadcasts `[Rel, LCi, Pi]`.  
4. **Receive Release:**  
   Process `Pj` removes `[Req, LC, Pi]` from its queue.  
5. **Grant Resource:**  
   `Pi` is granted the lock when:  
   - Its `[Req, LC, Pi]` is **at the head of the queue** (total order ⇒).  
   - `Pi` has received an **ack** from every other process with timestamp ≥ `LC`.  

## Instructions

- **N processes** are spawned and communicate via local sockets.
    - Each process has a unique ID in `[0, N-1]`.
    - Each process is given the same test file to execute.
    - The command line to launch a process is: `./process <id> <filename>`.
- **No local shared memory** is allowed (we simulate a distributed system).  
- Processes wait for each other, and compete to execute the `./critical` application provided, following orders from an input file.

The same input file is given to all spawned processes and has the following format:
```
N # number of processes participating in the algorithm
i Lock X # process i takes the lock and calls `./critical i X` to simulate a critical section lasting X seconds
i Wait j # process i waits for j to release a lock before doing its next Lock instruction
```

## Output Format
The `./critical` app appends to `log.txt` using the following format:
```
[Process \d+] [Time \d+] Lock taken
[Process \d+] [Time \d+] Lock released
```

## Example

With an input file such as:
```
2
0 Lock 1
1 Wait 0
0 Lock 1
1 Lock 1
```

A possible output is:
```
[Process 0] [Time 15568335] Lock taken
[Process 0] [Time 15568337] Lock released
[Process 1] [Time 15568338] Lock taken
[Process 1] [Time 15568347] Lock released
[Process 0] ...
```

⚠️ The actual output may vary due to scheduling.  
For instance, since `P0` does not wait for `P1`, it may reacquire the lock before `P1` manages to take it. Adding a `0 Wait 1` instruction before `0 Lock 1` would prevent this.

## Language

No restriction on the programming language. However, prefer one with: easy socket manipulation and simple concurrency primitives. (You will likely need multiple receiver threads and at least one sender thread handling shared structures.)

## Grading (Tentative)
- **Minimal pass:** All provided examples must run and be correct.  
- **OK mark (~13/20):** All tricky (not provided) test cases must run and be correct.  
- **Excellent (16+):** Write a report including:
  - **Performance:**  Measure maximum locks/second (without executing `./critical` and without printf to maximize performance and avoid external overheads).  
  - **Analysis:**  Use [`perf`](https://www.brendangregg.com/FlameGraphs/cpuflamegraphs.html) to generate a flamegraph. Where is the time spent? If you had to optimize, what would you do? (Open-ended, no need to implement).  

## Notes
- If using **TCP**, you may assume **FIFO message ordering**.  
- In a real life scenario, the `./process` would run forever but, for this assignment, it must terminate once all Lock instructions have been executed by all parties (hint: count the number of release messages vs. the number of Lock instructions).

