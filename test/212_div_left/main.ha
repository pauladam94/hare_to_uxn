fn main() void = {
	let a : u8 = 10 / 2 + 3; // should be 8 (not 6)
	let out : u8 = 0x18;
	*out = 48 + a; // print 8
	*out = a / 2 + 48 - 1; // print 3
	*out = '\n';
};
