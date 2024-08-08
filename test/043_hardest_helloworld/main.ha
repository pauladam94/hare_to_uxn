fn main() void = {
	let s = "hello world";
	let output = 0x18;
	for (let i = 0; i < 11; i += 1) {
		*output = s[i];
	}
};
