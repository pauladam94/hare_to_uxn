fn main() void = {
	let a : u8 = 3 + 10 / 2; // should be 8 (not 6)
	let out : u8 = 0x18;
	*out = 48 + a; // print 8
	*out = 48 + a / 2 - 1; // print 3
	*out = '\n';
};
