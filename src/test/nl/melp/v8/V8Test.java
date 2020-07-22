package nl.melp.v8;

import org.junit.Test;

import java.io.IOException;
import java.nio.file.Paths;

public class V8Test {
	@Test
	public void testV8() throws IOException {
		long runs = 1;
		long tStart = System.nanoTime();

		while (runs -- > 0) {
			V8.run(Paths.get("src/main/javascript/test.js"));
		}
		long tEnd = System.nanoTime();

		System.out.println(String.format("Run time: %.2fms", (float)(tEnd - tStart) / 1_000_000F));
	}
}
