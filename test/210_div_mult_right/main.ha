fn main() void = {
	let a : u8 = 2 + 10 / 2 - 2 * 2; // should be 3
	let out : u8 = 0x18;
	*out = 48 + a + 1; // print 4
	*out = 48 + a / 2; // print 1
	*out = '\n';
};
