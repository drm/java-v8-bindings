const call = (fn, args) => {
	const result = __do_call(JSON.stringify({fn, args}));
	if (!!result) {
		return JSON.parse(result);
	}
}

const print = (str) => {
	call("print", [str]);
}

print("Hello!");
