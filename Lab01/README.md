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
    - Each process is given the same test file as input for simplicity, but only executes its own commands (see examples below).
    - The command line to launch a process is: `./process <id> <test file>`.
- **No local shared memory** is allowed (we simulate a distributed system).  
- Processes wait for each other, and compete to execute the `./critical` application provided.

The same input file is given to all spawned processes and has the following format:
```
N # number of processes participating in the algorithm
i Lock X # process i takes the lock and calls `./critical i X` to simulate a critical section lasting X seconds
i Wait j # process i waits for j to release a lock before doing its next Lock instruction
i Wait j # process i waits again for j to release a lock before doing its next Lock instruction
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

⚠️ The actual output order may be different from the input order. In the previous example, P1 waits and then takes the lock before P0 reacquires it. Another valid output is P0 taking the lock twice before P1 acquires it.

The following is also a valid input:
```
2
0 Lock 1
1 Wait 0
1 Wait 0
0 Lock 1
1 Lock 1
```

In that case, P0 takes the lock twice before P1 takes the lock. Note that adding a third 'Wait' clause anywhere in the file should cause P1 to wait forever.
 

## Language

No restriction on the programming language. However, prefer one with: easy socket manipulation and simple concurrency primitives. (You will likely need multiple receiver threads and at least one sender thread handling shared structures.)

Regardless of the chosen programming language, your program MUST call the functions of hooks.c when accepting a connection, when connecting, and when sending or receiving data. If you use a programming language other than C, or an external library, make sure that it internally relies on `accept/connect/send/recv` to communicate with the outside world.

## Grading (Tentative)
- **Instant fail**: Git URL not provided on time, or code not accessible. Binary file committed (library or program). Code not compiling on a standard Linux distribution with basic dependencies intalled. Code not being able to be executed and checked automatically using `make check`. Modifying the `run.pl` or `hook.c` files in any way.
- **Pass**: all provided tests pass and the application makes no assumption except the fact that TCP sockets are FIFO and failure free.
- **Ok to good mark**: other tests (not provided) pass.
- **Excellent**: Measure maximum locks/second (without executing `./critical` and without the hooks to maximize performance and avoid external overheads) and compare with a standard lock implementation over shared memory. Discuss your findings in a report (to include in PDF format in your repo).

## Notes
- In a real life scenario, the `./process` would run forever but, for this assignment, it must terminate once all Lock instructions have been executed by all parties (hint: count the number of release messages received vs. the number of Lock instructions).

