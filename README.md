*This project has been created as part of the 42 curriculum by sdeppe*

# Codexion

## Description

This project consists of making multiple threads work together around some
shared data. We have n coders around a compiler; each coder wants to compile,
and for that it needs 2 dongles. Coder `i` can reach dongle `i - 1` and dongle
`i` (the first coder can access the last dongle too, so the dongles form a
circular list). Each coder runs on its own thread and cannot talk to the
other coders directly, but it can communicate with a shared scheduler thread
and a shared monitor thread. The scheduler arbitrates access to the dongles
according to a FIFO or EDF policy, and the monitor watches every coder's last
compile time to detect burnout.

## Instructions

For compilation you can run
``` bash
make all
```
or
``` bash
make codexion
```
or
``` bash
make
```

For recompiling the project you can do
``` bash
make re
```

For cleaning the object files you can do
``` bash
make clean
```

For compiling with the debug flag you can run
``` bash
make debug
```

There are two build modes, selected by which `src_files` the Makefile
compiles in:

- `make` / `make codexion` / `make debug`: the default build. All simulation
  messages go through a shared logging module that stores them and redraws a
  live ANSI table on stdout (owners, call counts, last log lines). Handy
  while developing, but every log call goes through one shared mutex, which
  adds measurable lock contention under load.
- `make release` / `make run`: swaps in a lightweight `run()`/`print()` pair
  (`error_manag_run_release.c` / `error_manag_print_release.c`) that calls
  the wrapped function directly (no mutex, no bookkeeping) and prints each
  formatted message straight to stdout as one plain line, closer to the
  reference log format in the subject. `make run` builds in this mode and
  launches it with the arguments in the `RUN` variable. Use this mode to
  actually judge timing-sensitive runs (tight burnout margins, many
  coders), since the debug table's overhead can by itself be enough to make
  a borderline test fail.

All program output goes to stdout; nothing is written to a log file.

## Resources

For this project I've been looking at:
- [learning the pthread lib](https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)
- [learning how to create a heap queue](https://www.youtube.com/watch?v=XycnarZEBvQ&t=519s)
- ChatGPT, used the way I'd use a search engine while actually writing code:
  quick syntax/API lookups, sanity-checking an approach, unblocking myself on
  a specific line. Not used to write whole functions for me.

### How AI (Claude) was used

Separately from the day-to-day ChatGPT lookups above, I used Claude (Claude
Code) in an interactive, iterative debugging session on top of an already
working implementation. I ran the tests, described the symptoms, and made
the design calls (e.g. choosing to keep a pairwise EDF comparison instead of
a heap-based priority queue); Claude investigated, proposed fixes, and I
reviewed/tested each one before it was applied. Concretely, across that
session it:

- Found and fixed a parsing bug where `compile_time`/`debug_time`/
  `refactor_time`/`required_compil`/`dungle_time` were all accidentally read
  from `argv[1]` instead of their own argument (copy-paste in `parser.c`).
- Found that `get_unsigned()` rejected legitimate `0` values (breaking the
  project's own default `dongle_cooldown=0`) and later added strict
  digit-only validation so non-numeric arguments (`"abc"`, `"12x"`) are
  rejected instead of silently parsed as garbage via `atoi`.
- Found and fixed a self-deadlock: with `n_cod == 1` (and, separately,
  `n_cod == 2`), the scheduler tried to lock the same dongle mutex twice in
  `set_fifo`/`set_edf` because a coder's left and right neighbour collapse
  onto the same index. Reproduced it with gdb thread backtraces, then fixed
  the scheduler so a single coder is correctly flagged as "can never get a
  second dongle" and dies without ever eating, instead of hanging forever.
- Found a stack-corruption bug (`sheduler_is_priority.c` declared a `char *`
  where `set_fifo`/`set_edf` wrote through it as `uint32_t *`, overwriting 3
  bytes past the real variable) and fixed the type mismatch.
- Used ThreadSanitizer to find a real lock-order inversion between
  `is_left_priority`/`is_right_priority` (both call into `set_edf`/
  `set_fifo`, which locked the two coders' mutexes in different orders
  depending on which neighbour was being checked); fixed by always locking
  the lower coder index first.
- Ran AddressSanitizer/UBSan and ThreadSanitizer sweeps across n_cod = 1, 2,
  4, 150 and 200 coders, FIFO and EDF, to check for leaks and data races.
- Added the `make release` build described above, after measuring that the
  debug table's per-log mutex contention was adding real latency under
  concurrency.
- Added a small grace period (`DEATH_GRACE_MS`) to the monitor's burnout
  check, so a dongle grant that was already legitimately in flight when a
  coder crossed its deadline gets a moment to land, while staying well
  inside the 10ms death-detection budget.
- Fixed several norminette violations (functions over the line limit, too
  many arguments/variables, a typedef declared in a `.c` file) introduced
  while restructuring the scheduler.
- Ran functional regression tests throughout (single coder, 2 coders, tight
  vs generous burnout margins, FIFO vs EDF, up to 200 coders) to check for
  starvation and correct liveness.

I did not let it touch the overall architecture decisions (the ring-sharing
model, the FIFO/EDF pairwise comparison instead of a heap) — those were
my calls, discussed and confirmed before any related code changed.

For traceability, the AI-assisted session started from commit `7571fde`
("cleanup"); everything from `723fe31` onward reflects that session, either
committed directly from it (`577215b`, `2fe59fa`, `24a389b`, `082a657`) or
committed by me after reviewing and testing changes worked out with it
(`723fe31`, `ea2af67`, `f4f46bf`). `16d580c` and `e2a832d` are my own
earlier parsing fixes, independent of it.

## Blocking cases handled

- **Deadlock**:
So the main way to block the code is to assign just one dongle to a coder:
if the second dongle is assigned to someone else, the two coders that need
access to that dongle block each other. If a full circle like that happens,
every coder ends up blocked. One concrete variant of this we hit: with
`n_cod == 1` (and `n_cod == 2`), a coder's left and right neighbour are the
same index, so naively locking "left" then "right" locks the same mutex
twice from the same thread. The scheduler now special-cases this instead of
attempting a real double-lock.

- **Coffman's conditions**:
The four classic conditions for deadlock are mutual exclusion (each dongle
mutex is held by at most one coder), hold-and-wait (a coder can be waiting
on one dongle while already holding its claim on another), no preemption
(a dongle is never forcibly taken away), and circular wait (a cycle of
coders each waiting on the next one's dongle). We can't remove the first
three without breaking the resource model, so the scheduler is the piece
that breaks circular wait: it only ever hands both of a coder's dongles to
it atomically, in one scheduling decision, instead of letting coders grab
one dongle and then block waiting for the second — so no coder is ever seen
holding one dongle while waiting on another.

- **Starvation prevention**:
The scheduler arbitrates every contested dongle with FIFO (earliest request
first) or EDF (earliest `last_compile_start + time_to_burnout` first), so a
coder that has been waiting the longest, or is closest to burning out, wins
the comparison against its direct neighbour. This was tested at scale: with
150 coders on FIFO and a `required_compiles` target, every single coder
ended up with exactly the same compile count, none left behind.

- **Cooldown handling**:
After a coder releases a dongle, the scheduler keeps it marked unavailable
until `dongle_cooldown` milliseconds have passed, checked against the
dongle's owner's last compile time under that coder's own mutex.

- **Precise burnout detection**:
A dedicated monitor thread, independent from the coders and the scheduler,
periodically recomputes how long until the next coder would burn out and
sleeps exactly that long (via `pthread_cond_timedwait` on an absolute
deadline) instead of polling. A small grace period is added on top of
`time_to_burnout` before a death is made final, so a dongle grant that was
already in flight when the deadline passed gets a chance to be recorded; the
announced death timestamp is still the real time the check ran, kept well
inside the 10ms precision the subject requires.

- **Log serialization**:
In the debug build, every log message is captured under one shared mutex
before being rendered, so two threads' messages can never interleave inside
the live table. In the release build, each message is first fully
assembled (text + substituted values) into a local buffer and only then
written out with a single `printf` call, which keeps that one call atomic
with respect to other threads' own single `printf` calls, instead of
interleaving character-by-character.

- **Mutex**:
When working with shared resources we need mutexes to guarantee no one
changes a value after it's been read by someone else: we lock access to a
variable, and we need to unlock it again at some point. That's where it's
easy to forget to unlock and block the whole runtime.

- **Infinite loops**:
It may seem like a very common mistake, but it's worth calling out here
because with multiple threads, debugging becomes a lot more subtle: one
thread can keep logging and look like everything is fine while another
thread is stuck in an infinite loop. In normal single-threaded C you can
just attach GDB and see the whole picture, but GDB doesn't watch every
thread at once — it only gives you a traceback once the program crashes (or
you break in manually and check each thread's backtrace yourself), which can
be confusing until you know to look for it.

## Thread synchronization mechanisms

So we have multiple tools to sync our threads: mutexes and condition
variables, used around a few specific pieces of shared state:

- **Mutex**:
A mutex is just a shared lock that says "this thread is changing a shared
variable", then "ok, done" so another thread can get access. In this
project, each coder has its own `can_take_mtx` protecting the dongle-request
state the coder, the scheduler and the monitor all read/write, and its own
`compile_done_time_mtx` protecting the timestamp of its last compile start.

- **Conds**:
A cond is attached to a mutex: a thread waits on it instead of spinning to
check whether a shared variable changed, and only wakes up once another
thread has changed that variable and broadcast the cond. Coders wait on
`can_take_cond` until the scheduler grants them a dongle pair; the scheduler
and the monitor both wait on their own conds and get woken up by coders as
soon as there's new state worth re-checking, instead of polling constantly.

- **Race example — death vs. starting to eat**:
The monitor reads a coder's `compile_done_time` to decide if it has burned
out, and the coder writes that same `compile_done_time` the moment it starts
a new compile. Both go through the same `compile_done_time_mtx`, so the
monitor can never read a half-updated timestamp, and a coder can't be judged
dead in the middle of recording that it just started eating — the two
operations are strictly ordered by that one mutex, not by hoping the timing
works out.
