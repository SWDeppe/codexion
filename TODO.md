# Codexion TODO - Multithreading Learning Path

> **Project Goal**: Simulate coders competing for limited USB dongles using POSIX threads, mutexes, and scheduling algorithms.
> Reference the subject file for full requirements.

---

## Phase 1: Project Setup & Basic Understanding

- [x] **1.1** Read the entire subject file carefully
  - Focus on sections: Overview (Chapter IV), Global rules (Chapter V), Mandatory part (Chapter VI)
  - Understand the simulation: coders → threads, dongles → shared resources, burnout → deadline
  
- [x] **1.2** Set up the project structure
  - Create `coders/` directory
  - Plan file organization (e.g., `main.c`, `coder.c`, `dongle.c`, `monitor.c`, etc.)
  - Decide on header files and data structures

- [x] **1.3** Create the Makefile skeleton
  - Rules: `NAME`, `all`, `clean`, `fclean`, `re`
  - Compiler flags: `-Wall -Wextra -Werror -pthread`
  - Test rule for running simple tests

- [ ] **1.4** Plan data structures
  - Sketch struct for `coder` (ID, state, timestamps, etc.)
  - Sketch struct for `dongle` (ID, owner, availability, etc.)
  - Sketch struct for simulation parameters and shared state

---

## Phase 2: Argument Parsing & Input Validation

- [ ] **2.1** Implement argument parser
  - Read all 8 arguments: `number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler`
  - Validate: no negatives, all integers, scheduler is "fifo" or "edf"
  - Print helpful error messages on invalid input

- [ ] **2.2** Test argument parsing
  - Write small test program: `test_args.c`
  - Test: valid inputs, missing arguments, negative numbers, invalid scheduler
  - Verify your error messages are clear

- [ ] **2.3** Store parameters in a structure
  - Create config/parameters struct to avoid global variables
  - Thread functions will receive this via void* arg

---

## Phase 3: Sequential Simulation (No Threads Yet)

- [ ] **3.1** Simulate a single coder without threading
  - Create basic loop: compile → debug → refactor → repeat
  - Use `usleep()` to simulate time delays
  - Print state changes (has taken dongle, is compiling, is debugging, is refactoring)
  - Track compile count

- [ ] **3.2** Test sequential flow
  - Write test program `test_sequential.c`
  - Run with hardcoded parameters (e.g., 1 coder, 200ms compile)
  - Verify timestamps and state transitions make sense
  - Keywords: `usleep()`, `gettimeofday()`, time calculation in milliseconds

- [ ] **3.3** Implement timestamp management
  - Create utility: `get_timestamp_ms()` using `gettimeofday()`
  - Create utility: `elapsed_ms(start_time)` to check burnout
  - Test these utilities independently

---

## Phase 4: Introduction to Mutexes

**New Concept: Protecting Shared State**

> A mutex (mutual exclusion) ensures only one thread accesses a resource at a time. This prevents race conditions.

- [ ] **4.1** Learn mutex basics
  - Keywords to research: `pthread_mutex_init()`, `pthread_mutex_lock()`, `pthread_mutex_unlock()`, `pthread_mutex_destroy()`, race condition, critical section
  - Read: POSIX pthreads documentation or Linux man pages

- [ ] **4.2** Write a test program for mutexes
  - Create `test_mutex.c`: two threads incrementing a shared counter
  - Run WITH mutex and WITHOUT mutex 100 times
  - Observe: with mutex = consistent results, without = corruption
  - Estimate how long this takes (goal: 30 min exploration)

- [ ] **4.3** Design dongle protection
  - Each dongle needs a mutex to protect its state (is_available, cooldown_end, etc.)
  - Sketch the mutex structure around dongle access

- [ ] **4.4** Implement basic mutex-protected dongle structure
  - Create dongle struct with `pthread_mutex_t lock`
  - Implement: `try_take_dongle()` and `release_dongle()` functions
  - Both functions must acquire the mutex

- [ ] **4.5** Test mutex-protected dongle (no threads yet)
  - Write `test_dongle.c`: simulate rapid take/release from sequential code
  - Verify locking/unlocking doesn't deadlock
  - Check dongle state remains consistent

---

## Phase 5: Introduction to Threads

**New Concept: Concurrent Execution**

> Threads allow multiple execution paths in one process. Each thread can run on a different CPU core.

- [ ] **5.1** Learn thread basics
  - Keywords to research: `pthread_create()`, `pthread_join()`, thread function signature, thread ID, detached vs joinable
  - Read: POSIX pthreads documentation

- [ ] **5.2** Write a simple thread test program
  - Create `test_threads.c`: spawn 3 threads that print their ID and sleep
  - Use `pthread_join()` to wait for completion
  - Keywords: thread spawning, joining, synchronization
  - Target time: 30 min to understand and test

- [ ] **5.3** Combine threads + mutexes
  - Write `test_thread_mutex.c`: multiple threads locking/unlocking the same mutex
  - Add delays inside critical section to test contention
  - Observe: threads wait their turn (mutex enforces mutual exclusion)

- [ ] **5.4** Create the basic coder thread function
  - Design: `void* coder_thread(void* arg)` 
  - Takes coder ID and shared parameters as argument
  - Implements the cycle: compile → debug → refactor → repeat
  - Currently ignores dongle acquisition (just simulate time)

- [ ] **5.5** Spawn multiple coder threads in main
  - Create N threads using `pthread_create()` in a loop
  - Pass each coder a unique ID and config struct
  - Use `pthread_join()` to wait for all threads
  - Test with 2–3 coders, short time values

---

## Phase 6: Introduction to Condition Variables

**New Concept: Efficient Waiting**

> Condition variables allow threads to sleep until a specific event occurs, rather than busy-waiting. More efficient than polling.

- [ ] **6.1** Learn condition variable basics
  - Keywords to research: `pthread_cond_init()`, `pthread_cond_wait()`, `pthread_cond_signal()`, `pthread_cond_broadcast()`, `pthread_cond_timedwait()`, spurious wakeup, predicate check
  - Read: POSIX pthreads condition variable documentation

- [ ] **6.2** Write a producer-consumer test with condition variables
  - Create `test_condition.c`: one producer thread, two consumer threads
  - Producer produces items, signals condition variable
  - Consumers wait on condition, process items
  - Keywords: wait-notify pattern, mutual exclusion with conditions
  - Target time: 30 min

- [ ] **6.3** Understand timed waits
  - Research: `pthread_cond_timedwait()`, absolute vs relative time, burnout deadline
  - Write `test_timedwait.c`: thread waits with a timeout, expires if condition not signaled
  - This is crucial for burnout detection

- [ ] **6.4** Design dongle queue with condition variables
  - Each dongle has: mutex, condition variable, waiting queue
  - Threads wait on the condition variable if dongle is unavailable
  - When dongle is released, broadcast to wake waiting threads

- [ ] **6.5** Implement dongle waiting queue
  - Create a simple queue (or priority queue stub for now)
  - When thread wants dongle: lock mutex, add to queue, wait on condition
  - When dongle freed: wake waiting threads, decide who gets it

---

## Phase 7: Implementing FIFO Scheduler

**New Concept: Fair Resource Allocation (FIFO)**

> FIFO = First In, First Out. The first thread to request a resource gets it.

- [ ] **7.1** Learn queue/FIFO concepts
  - Keywords to research: FIFO queue, enqueue, dequeue, fairness, starvation, request arrival time
  - Consider: linked list vs array-based queue for simplicity

- [ ] **7.2** Write a simple queue implementation test
  - Create `test_queue.c`: implement a basic queue (enqueue, dequeue)
  - Test with multiple threads pushing/popping
  - Keywords: thread-safe queue, mutex-protected operations

- [ ] **7.3** Design request tracking for FIFO
  - Each coder requesting a dongle gets a unique request ID or timestamp
  - Store: coder ID, request time, state
  - Decision rule: grant dongle to coder with earliest request time

- [ ] **7.4** Implement FIFO dongle allocation
  - When multiple coders wait for same dongle, pick the one with earliest request
  - Modify `try_take_dongle()` to check FIFO order
  - Release dongle, apply cooldown, then check queue for next coder

- [ ] **7.5** Test FIFO scheduler
  - Write `test_fifo_scheduler.c`: 3 coders, verify they acquire dongle in request order
  - Print: who requested when, who got dongle
  - Add delays to stagger requests and verify FIFO is respected

---

## Phase 8: Implementing EDF Scheduler

**New Concept: Priority-Based Scheduling (Earliest Deadline First)**

> EDF = Earliest Deadline First. Deadline = last_compile_start + time_to_burnout. Prioritize coders closest to burnout.

- [ ] **8.1** Learn EDF and priority scheduling
  - Keywords to research: Earliest Deadline First scheduling, real-time scheduling, priority queue, heap data structure, heap operations (insert, extract-min)
  - Understand: why EDF prevents starvation better than FIFO in tight deadlines

- [ ] **8.2** Write a simple priority queue (min-heap) test
  - Create `test_heap.c`: implement a min-heap or priority queue in C
  - Insert, extract-min, update-priority operations
  - Keywords: binary heap, heapify, parent/child indices
  - This may take longer than 30 min, so break into sub-tasks if needed

- [ ] **8.3** Understand heap vs queue tradeoffs
  - FIFO = simple array/linked list, O(1) insert, O(n) find-min
  - Heap = O(log n) insert/remove, more complex code
  - Decide: do you want a full heap or a simpler approach?

- [ ] **8.4** Design EDF request structure
  - Store: coder ID, last_compile_start, deadline = last_compile_start + time_to_burnout
  - Comparison function: compare deadlines, grant dongle to earliest deadline

- [ ] **8.5** Implement priority queue for EDF
  - Choose heap or simpler priority list (depends on coder count)
  - Insert request with deadline into queue
  - Extract coder with smallest (earliest) deadline

- [ ] **8.6** Implement EDF dongle allocation
  - When dongle released, find waiting coder with earliest deadline
  - Grant dongle, remove from queue, apply cooldown

- [ ] **8.7** Test EDF scheduler
  - Write `test_edf_scheduler.c`: 3 coders with different burnout times
  - Verify: coder closest to deadline gets dongle first
  - Compare EDF vs FIFO behavior: does EDF prevent burnout better?

---

## Phase 9: Dongle Cooldown Mechanism

**New Concept: Resource Unavailability Window**

> After a dongle is released, it cannot be taken again until cooldown_ms milliseconds pass. Prevents rapid reuse.

- [ ] **9.1** Understand cooldown purpose
  - Keywords to research: cooldown period, refractory period, resource availability window, timestamp tracking
  - Read subject: "dongle_cooldown (in milliseconds): After being released, a dongle is unavailable until its cooldown has passed."

- [ ] **9.2** Design cooldown tracking
  - Each dongle tracks: last_release_time, cooldown_duration
  - When requesting: check if current_time >= last_release_time + cooldown_duration
  - If not ready, thread must wait

- [ ] **9.3** Implement cooldown enforcement
  - Modify `can_take_dongle()` or `try_take_dongle()` to check cooldown
  - Threads wait (via condition variable) until cooldown expires
  - Keywords: absolute deadline, timed waiting, condition variable timeout

- [ ] **9.4** Test cooldown mechanism
  - Write `test_cooldown.c`: take dongle, release it, immediately try to take again
  - Verify: second take is blocked until cooldown expires
  - Measure timing: does it match the cooldown duration?

- [ ] **9.5** Integrate cooldown into scheduler
  - Modify FIFO and EDF to account for cooldown
  - When checking "who gets this dongle", also check if it's out of cooldown
  - Only grant to ready coders; others stay in queue

---

## Phase 10: Multiple Dongles & Circular Arrangement

**New Concept: Spatial Resource Topology**

> N coders, N dongles. Coder i has left dongle (i) and right dongle ((i+1) % N). Each coder needs 2 dongles.

- [ ] **10.1** Understand the circular dongle arrangement
  - Keywords to research: circular buffer, modulo arithmetic, deadlock in circular wait
  - Read subject: "There is one dongle between each pair of coders."

- [ ] **10.2** Design multi-dongle data structure
  - Create array of N dongle objects
  - Each coder calculates: left_dongle = coder_id, right_dongle = (coder_id + 1) % N
  - Store dongle array in shared config struct

- [ ] **10.3** Implement two-dongle acquisition
  - Coder acquires left dongle, then right dongle (or decide order)
  - Order matters for deadlock prevention: always acquire in same order (e.g., lower ID first)
  - Use scheduler (FIFO or EDF) for each dongle independently

- [ ] **10.4** Test multi-dongle acquisition without deadlock
  - Write `test_two_dongles.c`: 2-3 coders, each acquiring left and right
  - Verify: no deadlock, coders progress through compile → debug → refactor
  - Keywords: deadlock prevention, acquisition order, lock ordering

- [ ] **10.5** Stress test multi-dongle setup
  - Run with 4+ coders, short times, high compile requirements
  - Watch for deadlocks or delays
  - Tune scheduler and cooldown if needed

---

## Phase 11: Burnout Detection & Monitor Thread

**New Concept: Centralized Monitoring**

> Separate monitor thread detects if any coder hasn't started compiling within time_to_burnout ms.

- [ ] **11.1** Understand burnout concept
  - Keywords to research: deadline monitoring, polling vs event-driven, thread safety of shared state
  - Read subject: "If a coder did not start compiling within time_to_burnout milliseconds since the beginning of their last compile or the beginning of the simulation, they burn out."

- [ ] **11.2** Design burnout tracking
  - Each coder tracks: last_compile_start_time, deadline = last_compile_start + time_to_burnout
  - Monitor reads these timestamps periodically
  - If current_time > deadline, coder has burned out

- [ ] **11.3** Implement monitor thread
  - Create `monitor_thread()` function
  - Runs in a loop: check all coders' deadlines every ~10ms
  - If burnout detected: print message, set global "stop_simulation" flag

- [ ] **11.4** Implement shared "stop simulation" flag
  - Use mutex-protected boolean: `simulation_running`
  - Main thread checks this before spawning new cycles
  - Coder threads check this before attempting compile

- [ ] **11.5** Handle race conditions in burnout detection
  - Multiple threads access coder state: monitor thread, coder's own thread
  - Protect coder state with mutexes (per-coder or global?)
  - Ensure burnout message printed within 10 ms of actual burnout

- [ ] **11.6** Test burnout detection
  - Write `test_burnout.c`: 2 coders, force one into tight deadline
  - Verify: burnout message appears at correct time (~10 ms accuracy)
  - Test: burnout stops the simulation (other coders don't continue)

- [ ] **11.7** Test burnout prevention with good scheduling
  - Run with FIFO and EDF, verify EDF prevents burnout better
  - Experiment with dongle cooldown: shorter cooldown = coders can compile more frequently

---

## Phase 12: Synchronized Logging

**New Concept: Thread-Safe Output**

> All output must be serialized so messages don't interleave. Use a mutex to protect `printf()`/`write()`.

- [ ] **12.1** Understand logging requirements
  - Keywords to research: mutex-protected I/O, atomic writes, output serialization
  - Read subject: "A displayed state message should not be mixed up with another message."

- [ ] **12.2** Design logging mechanism
  - Create a mutex for output: `log_mutex`
  - All print statements: lock, print, unlock
  - Or write helper function: `log_message(const char* format, ...)`

- [ ] **12.3** Implement thread-safe logging
  - Create `print_log()` or similar that wraps `printf()` with mutex
  - Replace all `printf()` calls with safe version
  - Verify: even under heavy multithreading, logs don't interleave

- [ ] **12.4** Test concurrent logging
  - Write `test_logging.c`: multiple threads printing simultaneously
  - Verify: each message is on its own line, no garbled output
  - Keywords: mutual exclusion, critical section

- [ ] **12.5** Add precise timestamp logging
  - Print timestamp_ms before each message (as per spec)
  - Format: `timestamp coder_id message`
  - Example: `0 1 has taken a dongle`

---

## Phase 13: Integration & Full Simulation

**Milestone: Put It All Together**

- [ ] **13.1** Combine all components
  - Main: parse args → create config
  - Spawn monitor thread
  - Spawn N coder threads
  - Each coder: loop compile-debug-refactor with dongle acquisition
  - Monitor: detect burnout, stop simulation

- [ ] **13.2** Verify correct output format
  - Timestamps in milliseconds
  - State messages: "has taken a dongle", "is compiling", "is debugging", "is refactoring", "burned out"
  - One message per line, no interleaving

- [ ] **13.3** Test simulation end conditions
  - Stop if: any coder burns out, OR all coders completed required compiles
  - Verify: correct end condition is triggered

- [ ] **13.4** Run with provided example parameters
  - Subject provides example: number_of_coders=4, time_to_burnout=1500, time_to_compile=200, ...
  - Verify output roughly matches expected behavior

- [ ] **13.5** Test with FIFO scheduler
  - Run several times, verify no deadlock
  - Compare fairness: do all coders get equal access?

- [ ] **13.6** Test with EDF scheduler
  - Run several times, verify fairness and burnout prevention
  - Does EDF prevent burnout better than FIFO?

- [ ] **13.7** Test edge cases**
  - 1 coder, 1 dongle
  - Many coders (5+), tight deadlines
  - Zero cooldown
  - Very short burnout times
  - Very long compile times

---

## Phase 14: Memory Management & Cleanup

**New Concept: Resource Deallocation**

> All allocated memory must be freed. All mutexes and condition variables must be destroyed.

- [ ] **14.1** Identify all allocations
  - Dongles, coders, queues, config, any other structs
  - List every `malloc()` call

- [ ] **14.2** Plan cleanup order
  - Destroy condition variables and mutexes
  - Free each struct in reverse order of creation
  - Keywords: LIFO cleanup, resource acquisition is resource release (RAII concept)

- [ ] **14.3** Implement cleanup function
  - Create `cleanup_all()` function
  - Called after all threads join
  - Systematically free all resources

- [ ] **14.4** Test for memory leaks
  - Compile and run with `valgrind` or similar
  - Fix any leaks reported
  - Keywords: memory leak detection, heap profiling

- [ ] **14.5** Test cleanup under abnormal exit
  - Simulate early burnout (simulation stops)
  - Ensure cleanup still happens (maybe via signal handler or main cleanup at end)

---

## Phase 15: Code Quality & Norm Compliance

- [ ] **15.1** Check C norm compliance
  - Function length, line length, variable naming, indentation
  - Use norm checker if available (e.g., `norminette`)
  - Fix any violations

- [ ] **15.2** Organize code into multiple files
  - Suggested split: `main.c`, `coder.c`, `dongle.c`, `monitor.c`, `logging.c`, `utils.c`
  - Create corresponding header files: `*.h`
  - Makefile compiles all

- [ ] **15.3** Add comprehensive comments
  - Document thread functions, mutex/condition variable usage
  - Explain non-obvious logic (e.g., deadlock prevention strategy)

- [ ] **15.4** Review and refactor
  - Check for code duplication
  - Simplify complex functions
  - Ensure clarity

---

## Phase 16: README & Documentation

- [ ] **16.1** Write README.md
  - First line: italicized credit "This project has been created as part of the 42 curriculum by <login>"
  - Description: what the project does (resource competition simulation)
  - Instructions: how to compile and run
  - Resources: links to pthread docs, scheduling articles, etc.

- [ ] **16.2** Add "Blocking cases handled" section
  - Deadlock prevention (e.g., "always acquire in ascending dongle order")
  - Starvation prevention (e.g., "EDF scheduler ensures no coder is ignored indefinitely")
  - Cooldown handling
  - Precise burnout detection (monitor thread with ~10ms tolerance)
  - Log serialization (mutex-protected output)

- [ ] **16.3** Add "Thread synchronization mechanisms" section
  - Explain each mutex: what it protects
  - Explain each condition variable: what event it signals
  - Give example of how deadlock is prevented
  - Describe communication pattern between threads

- [ ] **16.4** List AI usage (if applicable)
  - Document which tasks AI assisted with (e.g., "brainstormed heap implementation", "reviewed deadlock prevention logic")
  - Be honest and specific

---

## Phase 17: Testing & Debugging

- [ ] **17.1** Create test scripts
  - Bash script to run simulation with various parameters
  - Capture output to files for comparison
  - Check for common issues: deadlock, early burnout, memory leaks

- [ ] **17.2** Test with stress scenarios
  - Many coders (10+)
  - Extreme cooldowns (very short or very long)
  - Tight burnout windows
  - Asymmetric timing (very fast compile vs slow debug)

- [ ] **17.3** Debug with thread-aware tools (optional)
  - Use `gdb` with `info threads` to inspect thread state
  - Use `valgrind --tool=helgrind` to detect data races
  - Keywords: thread debugging, race condition detection

- [ ] **17.4** Verify scheduler fairness
  - Log compile counts per coder
  - Ensure no coder is starved under FIFO
  - Ensure EDF doesn't starve in feasible scenarios

- [ ] **17.5** Final validation
  - Run multiple times, check for non-deterministic behavior
  - Compare FIFO vs EDF outputs on same parameters
  - Ensure burnout messages are consistent and timely

---

## Phase 18: Bonuses (Optional)

- [ ] **18.1** Explore bonus features
  - Subject may not mandate specific bonuses
  - Possible ideas: visualization, detailed statistics, alternative schedulers
  - Check subject's bonus section before implementing

- [ ] **18.2** Advanced scheduler (if bonus)
  - Implement a third scheduler (e.g., Round-Robin, Priority-based)
  - Ensure it's in separate `*_bonus.c` files

- [ ] **18.3** Enhanced monitoring (if bonus)
  - Track metrics: average compile count, fairness index, total simulation time
  - Print summary stats at end

---

## Quick Reference: Test Program Checklist

Use this to track mini-test programs you create:

- [ ] `test_args.c` — Argument parsing validation
- [ ] `test_sequential.c` — Single coder, no threads, basic state machine
- [ ] `test_mutex.c` — Mutex protection, race condition demo
- [ ] `test_dongle.c` — Mutex-protected dongle, no threads
- [ ] `test_threads.c` — Basic thread spawning and joining
- [ ] `test_thread_mutex.c` — Threads + mutexes combined
- [ ] `test_condition.c` — Producer-consumer with condition variables
- [ ] `test_timedwait.c` — Timed wait (for burnout detection)
- [ ] `test_queue.c` — FIFO queue implementation
- [ ] `test_fifo_scheduler.c` — FIFO resource allocation
- [ ] `test_heap.c` — Priority queue / min-heap (for EDF)
- [ ] `test_edf_scheduler.c` — EDF resource allocation
- [ ] `test_cooldown.c` — Dongle cooldown window
- [ ] `test_two_dongles.c` — Multi-dongle acquisition, no deadlock
- [ ] `test_burnout.c` — Burnout detection and monitoring
- [ ] `test_logging.c` — Thread-safe logging
- [ ] `test_valgrind.c` — Memory leak check on main program

---

## Keywords & Resources to Google

### General Threading:
- POSIX pthreads tutorial
- Thread synchronization primitives
- Race conditions in multithreading
- Thread-safe data structures

### Mutexes:
- pthread_mutex_lock, pthread_mutex_unlock
- Mutex deadlock prevention
- Lock ordering discipline

### Condition Variables:
- pthread_cond_wait, pthread_cond_timedwait
- Spurious wakeups
- Predicate checks with condition variables

### Scheduling:
- FIFO scheduling algorithm
- Earliest Deadline First (EDF) real-time scheduling
- Priority queues and heaps in C

### Data Structures:
- Binary heap implementation in C
- Queue (linked list vs array)
- Priority queue (heap-based)

### Deadlock Prevention:
- Coffman's four conditions for deadlock
- Lock ordering strategy
- Resource hierarchy

### Burnout / Deadline:
- Real-time task deadline monitoring
- gettimeofday() and time measurement
- Watchdog / monitor thread pattern

### Tools:
- gdb for thread debugging
- valgrind for memory leaks
- helgrind for race condition detection

---

## Notes & Reflections

Use this section to log your learning and design decisions:

- [ ] Decided on data structure for X (add notes)
- [ ] Found deadlock issue with Y (add notes)
- [ ] Scheduler fairness test results (add notes)
- [ ] Performance observations (add notes)

---

**Good luck! Remember: start small, test each piece, and don't hesitate to refactor when you learn a better approach.**
