package nl.melp.v8;

public class V8 {
	static {
		System.loadLibrary("v8bindings");
	}

	public static void main(String[] args) {
		new V8().sayHello();
	}

	// Declare a native method sayHello() that receives no arguments and returns void
	private native void sayHello();
}
