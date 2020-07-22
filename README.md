# Java to V8 Bindings - the simple way.

The idea is loosely based on [J2V8](https://github.com/eclipsesource/J2V8). 
But since I want to know how this stuff _really_ works, I started binding 
[V8](https://v8.dev) with Java through JNI from scratch.

The current state is no more than a proof of concept: running a script and
calling a java method from JS. The idea is that a simple message-passing
protocol is implemented, all data using JSON as the carrier. A call from 
V8 to Java consists of a JSON message containing `fn` and `args`, where
the `fn` is the function name registered in `nl.melp.v8.V8` and 

I want to keep the logic for managing the contexts and isolates as much
in C++ as possible to avoid cluttering the Java code with it, so this
will probably evolve into something that uses an internal handle of 
some sorts (incremental id?) for the isolates and contexts. However,
I'm not convinced that it's actually necessary to have more than one
isolate; this will probably be based on some performance tests.

## TODO

- [ ] Isolate and context startup data
- [ ] ThreadLocal support
- [ ] Debugger support. 
