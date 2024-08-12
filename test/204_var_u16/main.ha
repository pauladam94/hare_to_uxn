fn main() void = {
    let a : u16 = 10;
    let output : u8 = 0x18;
    *output = 48 + 1;// print 1
    *output = a; 
};
