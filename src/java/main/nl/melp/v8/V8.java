package nl.melp.v8;

public class V8 {
	static {
		System.loadLibrary("v8bindings");
	}

	public static void main(String[] args) {
		final V8 v8 = new V8();
		v8.sayHello();
		v8.sayHello(args[0]);
	}

	// Declare a native method sayHello() that receives no arguments and returns void
	private native void sayHello();
	private native void sayHello(String name);
}
