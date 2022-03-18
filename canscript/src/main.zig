const std = @import("std");
const Allocator = std.mem.Allocator;
const lua = @import("lua.zig").lua;

const erlcmd = @cImport({
    @cInclude("erlcmd.h");
});

pub const Opcode = enum {
    Set,
    Get,
    LoadFile,
    Pcallk,
};

pub const Key = enum { RPM };

pub const VType = enum {
    d8,
    s8,
    df64,
};

pub const Value = union(VType) { d8: u8, s8: i8, df64: f64 };

pub const Container = struct {
    allocator: Allocator,
    L: ?*lua.lua_State,
    data: []Value,
    pub fn init(allocator: Allocator) !Container {
        var L = lua.luaL_newstate();
        errdefer lua.lua_close(L);

        lua.luaL_openlibs(L);

        const data: []Value = try allocator.alloc(Value, 200);
        errdefer allocator.free(data);

        return Container{ .allocator = allocator, .L = L, .data = data };
    }

    pub fn deinit(self: *Container) void {
        lua.lua_close(self.L);
        self.allocator.free(self.data);
    }
};

pub fn erlcmd_handle(len: usize, req: [*c]const u8, cookie: ?*anyopaque) callconv(.C) void {
    const container = @ptrCast(*const Container, @alignCast(8, cookie));
    const opcode = @intToEnum(Opcode, req[0]);
    // 0=opcode
    // 1=key
    // 2=type
    // 3=value
    _ = len;
    switch (opcode) {
        Opcode.Set => {
            const value = switch (@intToEnum(VType, req[2])) {
                VType.d8 => Value{ .d8 = @bitCast(u8, req[3]) },
                VType.s8 => Value{ .s8 = @bitCast(i8, req[3]) },
                VType.df64 => Value{ .df64 = @bitCast(f64, req[3..11].*) },
            };
            container.data[req[1]] = value;
            std.log.info("set {d}({d})", .{ req[1], req[2] });
        },
        Opcode.Get => {
            // uint16 size|opcode|key|type
            const header = [_]u8{ 0, 0, req[0], req[1], 255 };
            var payload = container.allocator.alloc(u8, header.len + @sizeOf(Value)) catch return;
            defer container.allocator.free(payload);

            std.mem.set(u8, payload, 0);
            std.mem.copy(u8, payload, &header);

            const t: usize = switch (container.data[req[1]]) {
                VType.d8 => |value| blk: {
                    payload[4] = @enumToInt(VType.d8);
                    payload[5] = value;
                    break :blk @sizeOf(u8);
                },
                VType.s8 => |value| blk: {
                    payload[4] = @enumToInt(VType.s8);
                    payload[5] = @bitCast(u8, value);
                    break :blk @sizeOf(i8);
                },
                VType.df64 => |value| blk: {
                    payload[4] = @enumToInt(VType.df64);
                    var real_value = @bitCast([8]u8, value);
                    std.mem.copy(u8, payload[header.len..], &real_value);
                    break :blk @sizeOf(f64);
                },
            };
            erlcmd.erlcmd_send(payload.ptr, header.len + t);
        },
        Opcode.LoadFile => {
            // uint16 size|opcode|status
            var header = [_]u8{ 0, 0, req[0], 0 };

            const load_status = lua.luaL_loadfilex(container.L, req[1..len].ptr, null);
            if (load_status != lua.LUA_OK) {
                const string = std.fmt.allocPrint(container.allocator, "{s}", .{lua.lua_tolstring(container.L, -1, null)}) catch return;
                defer container.allocator.free(string);

                var payload = container.allocator.alloc(u8, header.len + string.len) catch return;
                defer container.allocator.free(payload);

                std.mem.set(u8, payload, 0);
                std.mem.copy(u8, payload, &header);
                std.mem.copy(u8, payload[header.len..], string);
                payload[3] = 1;
                erlcmd.erlcmd_send(payload.ptr, payload.len);
                return;
            } else {
                erlcmd.erlcmd_send(&header, header.len);
            }
        },
        Opcode.Pcallk => {
            var header = [_]u8{ 0, 0, req[0], 0 };

            const call_status = lua.lua_pcallk(container.L, 0, lua.LUA_MULTRET, 0, 0, null);
            if (call_status != lua.LUA_OK) {
                const string = std.fmt.allocPrint(container.allocator, "{s}", .{lua.lua_tolstring(container.L, -1, null)}) catch return;
                defer container.allocator.free(string);

                var payload = container.allocator.alloc(u8, header.len + string.len) catch return;
                defer container.allocator.free(payload);

                std.mem.set(u8, payload, 0);
                std.mem.copy(u8, payload, &header);
                std.mem.copy(u8, payload[header.len..], string);
                payload[3] = 1;
                erlcmd.erlcmd_send(payload.ptr, payload.len);
                return;
            } else {
                erlcmd.erlcmd_send(&header, header.len);
            }
        },
    }
}

// https://stackoverflow.com/questions/11689135/share-array-between-lua-and-c

pub fn main() anyerror!void {
    var con = try Container.init(std.heap.c_allocator);
    defer con.deinit();

    var erlcmd_handler: erlcmd.erlcmd = undefined;
    erlcmd.erlcmd_init(&erlcmd_handler, erlcmd_handle, &con);

    while (true) {
        erlcmd.erlcmd_process(&erlcmd_handler);
    }
}
