fn main() void = {
	let a : u8 = 10 / 2 - 2 * 2 + 2; // should be 8
	let out : u8 = 0x18;
	*out = 48 + a + 1; // print 4
	*out = a / 2 + 48; // print 1
	*out = '\n';
};
