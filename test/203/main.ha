fn main() void = {
    let a : u8 = 4;
    let b : u8 = 2;
    *24 = 48 + a; // print '4'
    *24 = 48 + b; // print '2'
    *24 = 48 + b + 2 + 3; // print '7'
    *24 = 48 - 2 + a; // print '2'
    *24 = 10; // print '\n'
};
