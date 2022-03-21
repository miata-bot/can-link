const std = @import("std");

const erlcmd = @cImport({
    @cInclude("erlcmd.h");
});

const gps = @cImport({
    @cInclude("gps.h");
});

const poll = @cImport({
    @cDefine("_GNU_SOURCE", "1");
    @cInclude("poll.h");
});

pub fn erlcmd_handle(len: usize, req: [*c]const u8, cookie: ?*anyopaque) callconv(.C) void {
    _ = len;
    _ = req;
    _ = cookie;
}

pub fn main() anyerror!void {
    const allocator = std.heap.c_allocator;

    var gps_data: gps.gps_data_t = undefined;
    defer _ = gps.gps_close(&gps_data);

    var rc = gps.gps_open("localhost", "2947", &gps_data);
    if (rc != 0)
        return;

    var erlcmd_handler: erlcmd.erlcmd = undefined;
    erlcmd.erlcmd_init(&erlcmd_handler, erlcmd_handle, null);

    _ = gps.gps_stream(&gps_data, gps.WATCH_ENABLE | gps.WATCH_JSON, null);
    while (gps.gps_waiting(&gps_data, 5000000)) main_wait: {
        var fdset: [1]poll.pollfd = undefined;
        fdset[0].fd = 3;
        fdset[0].events = poll.POLLIN;
        fdset[0].revents = 0;
        rc = poll.poll(&fdset, 1, 0);
        if(rc < 0) {
            if(std.os.errno(0) == std.os.linux.E.INTR)
                continue;
            std.log.err("poll", .{});
            break :main_wait;
        }

        if((fdset[0].revents & (poll.POLLIN | poll.POLLHUP) > 0)) {
            std.log.info("poll", .{});
            erlcmd.erlcmd_process(&erlcmd_handler);
        }

        if (gps.gps_read(&gps_data, null, 0) == -1) {
            std.log.err("gps_read", .{});
            break :main_wait;
        }

        // did not even get mode, nothing to see here
        if ((gps_data.set & gps.MODE_SET) == gps.MODE_SET) {
            std.log.info("\r\nfix mode: {d}", .{gps_data.fix.mode});
            const header = [_]u8{ 0, 0, 1 };
            var payload = allocator.alloc(u8, header.len + @sizeOf(gps.gps_data_t)) catch break :main_wait;
            defer allocator.free(payload);

            std.mem.set(u8, payload, 0);
            std.mem.copy(u8, payload, &header);
            var tmp = @bitCast([@sizeOf(gps.gps_data_t)] u8, gps_data);
            std.mem.copy(u8, payload[header.len..],  &tmp);
            erlcmd.erlcmd_send(payload.ptr, payload.len);
        } else {
            continue;
        }
    }
}
