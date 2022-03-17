const std = @import("std");
const Allocator = std.mem.Allocator;

pub const Key = enum {
    RPM
};

pub const Value = union {
    df64: f64
};

pub const Container = struct {
    allocator: Allocator,
    data: []Value,
    pub fn init(allocator: Allocator) !Container {
        const data: []Value = try allocator.alloc(Value, 200);
        errdefer allocator.free(data);

        return Container {
            .allocator = allocator,
            .data = data
        };
    }

    pub fn deinit(self: *const Container) void {
        self.allocator.free(self.data);
    }
};

// https://stackoverflow.com/questions/11689135/share-array-between-lua-and-c

pub fn main() anyerror!void {
    const con = try Container.init(std.heap.c_allocator);
    defer con.deinit();
    con.data[@enumToInt(Key.RPM)].df64 = 12.5;
    std.log.info("All your codebase are belong to us. {d}", .{con.data[@enumToInt(Key.RPM)].df64});
}
