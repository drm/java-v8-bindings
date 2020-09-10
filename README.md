
**Obsolete, see [mv8](http://github.com/drm/mv8) (which is a fork of 
[martijnvogten/mv8](http://github.com/martijnvogten/mv8)) 
for a complete implementation**

# Java to V8 Bindings - Proof of Concept

The idea is loosely based on [J2V8](https://github.com/eclipsesource/J2V8). 
But since I want to know how this stuff _really_ works, I started binding 
[V8](https://v8.dev) with Java through JNI from scratch.

This project is no more than a proof of concept: running a script and
calling a java method from JS. The idea is that a simple message-passing
protocol is implemented, all data using JSON as the carrier. A call from 
V8 to Java consists of a JSON message containing `fn` and `args`, where
the `fn` is the function name registered as a callback.
