package nl.melp.v8;


import com.google.gson.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

public class V8 {
	private static final Logger logger = LoggerFactory.getLogger(V8.class);
	private static final JsonParser parser = new JsonParser();
	private static final Gson gson = new Gson();
	private static final Map<String, Method> bindings = new HashMap<>();

	public static void print(String s) {
		System.out.println(">>> " + s);
	}

	static {
		try {
			bindings.put("print", V8.class.getDeclaredMethod("print", String.class));
		} catch (NoSuchMethodException e) {
			throw new RuntimeException(e);
		}
	}

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
		JsonObject root = parser.parse(message).getAsJsonObject();
		String fn = root.get("fn").getAsString();
		if (!bindings.containsKey(fn)) {
			// TODO use custom class?
			throw new RuntimeException("Invalid reference to global binding: " + fn);
		}
		JsonArray args = root.get("args").getAsJsonArray();
		Object[] a = new Object[args.size()];
		for (int i = 0; i < args.size(); i ++) {
			a[i] = args.get(i).getAsString();
		}
		Method m = bindings.get(fn);
		Object ret;
		try {
			ret = m.invoke(null, a);
		} catch (IllegalAccessException | InvocationTargetException e) {
			e.printStackTrace();
			// TODO throw?
			return "null";
		}
		return gson.toJson(ret);
	}

	public static synchronized native void run(String script);
//	public static native String send(String message);
}
