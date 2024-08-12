fn main() void = {
	let a : u8 = 2 * 3 + 3; // should be 9 (not 15)
	let out : u8 = 0x18;
	*out = 48 + a; // print 9
	*out = 2 * 2 + 48 + a - 5; // print 8
	*out = '\n';
};
