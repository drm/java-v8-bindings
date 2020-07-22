#!/usr/bin/env bash

export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"
set -e
[ "$VERBOSE" ] && set -x

clean() {
	rm -rf ./bin/
}

headers() {
	(
		cd src/java/main;
		javac -h ../../cpp/include nl/melp/v8/V8.java
	)
}

compile-cpp() {
	mkdir -p bin/cpp/main
	(
		cd src/cpp/main;
		g++ -c -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux v8.cpp -o ../../../bin/cpp/v8.o
		g++ -shared -fPIC -o ../../../bin/cpp/libv8bindings.so ../../../bin/cpp/v8.o -lc
	)
}

compile-java() {
	mkdir -p bin/java/main
	(
		cd src/java/main;
		javac nl/melp/v8/V8.java -d ../../../bin/java/main;
	)
}

compile() {
	compile-cpp;
	compile-java;
}

hello() {
	java -cp bin/java/main -Djava.library.path=$(cd bin/cpp && pwd) nl.melp.v8.V8
}

for t in $@; do
	$t
done
