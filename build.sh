#!/usr/bin/env bash

export JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64"
export V8_HOME="/home/gerard/git/v8"
export V8_RELEASE="x64.release.sample"

set -e
[ "$VERBOSE" ] && set -x

clean() {
	rm -rf ./bin/
}

compile-cpp() {
	mkdir -p bin/

#	g++ -c -fPIC \
#		-I${JAVA_HOME}/include \
#		-I${JAVA_HOME}/include/linux \
#		-I${V8_HOME}/include \
#		-o bin/v8.o
#
#
 #		-Wl,--whole-archive ${V8_HOME}/out.gn/${V8_RELEASE}/obj/libv8_monolith.a -Wl,--no-whole-archive \


	# Build shared (.so), including the v8 monolith
	g++ -shared \
		-I${JAVA_HOME}/include \
		-I${JAVA_HOME}/include/linux \
		-I${V8_HOME}/include \
		-o bin/libv8bindings.so \
		src/main/cpp/v8.cc \
		-Wl,${V8_HOME}/out.gn/${V8_RELEASE}/obj/libv8_monolith.a \
		-ldl -pthread -std=c++11 -fPIC

#	g++ \
#		./samples/shell.cc \
#		-I${V8_HOME} \
#		-I${V8_HOME}/include \
#		-lv8_monolith -L${V8_HOME}/out.gn/${V8_RELEASE}/obj/ \
#		-pthread \
#		-o ./shell
#
#	g++ \
#		./samples/exec.cc \
#		-I${V8_HOME} \
#		-I${V8_HOME}/include \
#		-lv8_monolith -L${V8_HOME}/out.gn/${V8_RELEASE}/obj/ \
#		-pthread \
#		-o ./exec
}

compile-exec() {
	g++ \
		./samples/exec.cc \
		-I${V8_HOME} \
		-I${V8_HOME}/include \
		-lv8_monolith -L${V8_HOME}/out.gn/${V8_RELEASE}/obj/ \
		-pthread \
		-o ./exec
}

compile-java() {
	mkdir -p bin/
	javac src/main/java/nl/melp/v8/V8.java -d bin -h src/main/cpp/include;
}

compile() {
	compile-java;
	compile-cpp;
}

hello() {
	java -cp bin/ -Djava.library.path=$(cd bin && pwd) nl.melp.v8.V8 $(id -un)
}

build-all() {
	clean
	compile
	hello
}

if [ "$#" -gt 0 ]; then
	for t in $@; do
		$t
	done
else
	build-all;
fi
