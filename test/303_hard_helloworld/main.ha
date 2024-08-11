fn main() void = {
    let s = "hello world\n";
    let output = 0x18;
    for (let i = 0; i < 12; i += 1) {
        *output = s[i];
    }
};
