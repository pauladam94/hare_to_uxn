fn main() void = {
    let out : u8 = 0x18;
    if (12 + 3 > 14) {
        *out = 48 + 3;
    };

    *out = 'b';
    *out = '\n';

};
