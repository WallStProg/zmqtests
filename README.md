# ZeroMQ test programs

## threads

The sample programs here demonstrate several different approaches to inter-thread signalling with ZeroMQ, and show which work and which don't in "real-world" scenarios.

> By "real-world" in this case I mean the following:
> 
> - The code supports an arbitrary number of threads;
> - The code supports arbitrary numbers of messages;
> - Messages can be sent at any time;
> - The code is likely to be implemented as a library which provides an interface between ZeroMQ and the application.  In this scenario, the library code (the code calling ZeroMQ functions) cannot make any assumptions about its context.

