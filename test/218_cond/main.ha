fn main() void = {
	let out : u8 = 0x18;
	let x : u8 = 12;
	if (x > 2) {
		*out = 48 + 1;
	} else {
		if (x <= 2) {
			*out = 48 + 2;
		};
	};
	*out = 'z';
	*out = '\n';
};
