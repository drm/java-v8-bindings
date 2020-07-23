package nl.melp.v8;


import com.google.gson.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

public class V8 {
	private static final Logger logger = LoggerFactory.getLogger(V8.class);
	private static final JsonParser parser = new JsonParser();
	private static final Gson gson = new Gson();
	private static final Map<String, Method[]> bindings = new HashMap<>();

	public static void print(String s) {
		System.out.println(">>> " + s);
	}
	public static void print(Number s) {
		System.out.println("num: " + s);
	}
	public static void print(Boolean s) {
		System.out.println("bool: " + s);
	}

	static {
		try {
			bindings.put(
				"print", new Method[]{
					V8.class.getDeclaredMethod("print", String.class),
					V8.class.getDeclaredMethod("print", Number.class),
					V8.class.getDeclaredMethod("print", Boolean.class)
				}
			);
		} catch (NoSuchMethodException e) {
			throw new RuntimeException(e);
		}
	}

	static {
		System.loadLibrary("v8bindings");
	}

	public static void run(Path path) throws IOException {
		run(Files.readString(path));
	}

	public static Method findMatchingMethod(String functionName, Object[] args) throws NoSuchMethodException {
		if (!bindings.containsKey(functionName)) {
			throw new NoSuchMethodException("There is no implementation of function " + functionName);
		}
		final Method[] methods = bindings.get(functionName);
		for (Method m : methods) {
			// TODO: implement varargs?
			if (m.getParameterCount() == args.length) {
				boolean isMatch = true;
				final Class<?>[] parameterTypes = m.getParameterTypes();
				for (int i = 0; i < args.length; i ++) {
					if (!parameterTypes[i].isAssignableFrom(args[i].getClass())) {
						isMatch = false;
						break;
					}
				}

				if (isMatch) {
					return m;
				}
			}
		}
		throw new NoSuchMethodException("There is no method available matching the parameter types");
	}


	public static synchronized String recv(String message) throws NoSuchMethodException {
		JsonObject root = parser.parse(message).getAsJsonObject();
		String fn = root.get("fn").getAsString();
		if (!bindings.containsKey(fn)) {
			// TODO use custom class?
			throw new RuntimeException("Invalid reference to global binding: " + fn);
		}
		JsonArray args = root.get("args").getAsJsonArray();
		Object[] a = new Object[args.size()];
		for (int i = 0; i < args.size(); i ++) {
			if (args.get(i).isJsonObject()) {
				a[i] = args.get(i).getAsJsonObject();
			} else if (args.get(i).isJsonArray()) {
				// Do we want this to become an Object[] ?
				a[i] = args.get(i).getAsJsonArray();
			} else if (args.get(i).isJsonNull()) {
				a[i] = null;
			} else if (args.get(i).isJsonPrimitive()) {
				final JsonPrimitive primitive = args.get(i).getAsJsonPrimitive();
				if (primitive.isNumber()) {
					a[i] = primitive.getAsDouble();
				} else if (primitive.isString()) {
					a[i] = primitive.getAsString();
				} else if (primitive.isBoolean()) {
					a[i] = primitive.getAsBoolean();
				} else {
					throw new IllegalStateException("This element is of unknown type!");
				}
			} else {
				throw new IllegalStateException("This element is of unknown type!");
			}
		}
		Method m = findMatchingMethod(fn, a);
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
