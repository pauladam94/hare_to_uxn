fn main() void = {
    let a : u8 = 4;
    let b : u8 = 2;
    *24 = 48 + a; // print '4'
    *24 = 48 + b; // print '2'
    b = b + 2 + 3;
    *24 = 48 + b; // print '7'
    a = a - 2;
    *24 = 48 + a; // print '2'
    *24 = 10; // print '\n'
};
