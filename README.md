*This project has been created as part of the 42 curriculum by sdeppe*

# Codexion  

## Desciption

This project consist of making multiple thread working together to work with some shared data
We have n coder around a compiler each coder wants to compile, for this purpose he needs 2 dungles.
Each coder can reach the dungle n - 1 and n the first coder can access to the last dungle (circular list)
The coders get a thread each anf cannot interact beetween eachother but can communicates with the main sheduler.
The sheduler has to make a priority queue dependent on FIFO or EDF to give access to the dungles.

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

for cleaning the object files you can do
``` bash
make clean
```

for compiling with the debug flag you can run 
``` bash
make debug
```

## Resources

For this project i've been looking at:
(https://www.youtube.com/playlist?list=PLfqABt5AS4FmuQf70psXrsMLEDQXNkLq2)[learning the pthread lib]
(https://www.youtube.com/watch?v=XycnarZEBvQ&t=519s)[learning how to create a heap queue] + ChatGPT for debuging purpose

## Blocking cases handled

- **Deadlock**:
So the main way to block the code is to assigne juste one dungle to a coder if the second dungle is assigned to someone else then the two coders that need to access this dungle will be blocked
If a full circle like that happens then all coder will ne blocked

- **Mutex**:
When working with shared ressources we need mutexes that are here to garenty no one changes the value read after being read by someone else so we block the access to a variable but also need to unlock it at some point. That's where we can forget to unlock it and block the whole runtime.

- **Infinite loops**:
It may seem's like a very common mistake but i want to add it here cause when working with multiple threads debuging becomes way more subtile cause one thread could be running fine and givving you logs like everything would be working as usual but another thread can be blocked in an infinite loop.
While in normal c programming we could juste look at GDB (or any other debugger) and say that everything work, but when it comes to multithreading gdb dosen't look at every thread at the same time juste gives a traceback once the programme crashes what can confuse a lot.  

## Thread synchronization mechanisms

So we have multiple tools to sync out threads like, as talk before, mutexes:
- **Mutex**:
A mutex is juste a shared pointer that says this thread is changing a shared variable then saying ok done and then another thread can get access

- **Condes**:
A cond is connected to a mutex its basicely waiting that another thread changes a shared variable locked by a mutex and once it get breadcasted it lookes at the variable instead of running come code that takes ressources to check if the variable changes the main purpose is also that if thread is waiting that a value chages is has to lock and unlock a lot a mutex but this makes the thread trying to change the value wait to get access but with a cond the mutex is not being locked 

