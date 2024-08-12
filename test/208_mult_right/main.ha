fn main() void = {
	let a : u8 = 3 + 2 * 3; // should be 9 (not 15)
	let out : u8 = 0x18;
	*out = 48 + a; // print 9
	*out = 48 + a - 5 + 2 * 2; // print 8
	*out = '\n';
};
