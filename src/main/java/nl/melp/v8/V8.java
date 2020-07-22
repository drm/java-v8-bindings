package nl.melp.v8;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class V8 {
	private static Logger logger = LoggerFactory.getLogger(V8.class);

	static {
		System.loadLibrary("v8bindings");
	}

	public static void main(String[] args) throws IOException {
		run(Paths.get("src/main/javascript/test.js"));
	}

	public static void run(Path path) throws IOException {
		run(Files.readString(path));
	}

	public static synchronized String recv(String message) {
		logger.debug("Got message from v8: " + message);
		return "{\"wie\": \"Je Moeder!\"}";
	}

	public static synchronized native void run(String script);
//	public static native String send(String message);
}
