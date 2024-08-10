fn main() void = {
    let a : u8 = test();
};

fn test() u8 = {
    main();
    return 42;
};
