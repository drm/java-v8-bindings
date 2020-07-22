const call = (fn, args) => {
	try {
		const result = __do_call(JSON.stringify({fn, args}));
		if (!!result) {
			return JSON.parse(result);
		}
	} catch (e) {
		const error = new Error(e);
		Error.captureStackTrace(error, call);
		throw error;
	}
}

const print = (str) => {
	call("print", [str]);
}

let i;
for (i = 0; i < 10000; i ++) {
	print("Hello!");
}
