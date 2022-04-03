const std = @import("std");
const iio = @cImport(
    {
        @cInclude("iio.h");
    }
);

const Device = struct {
    data: ?*iio.iio_device,
    name: []u8
};

pub fn main() anyerror!void {
    const allocator = std.heap.c_allocator;
    const context: ?*iio.iio_context = iio.iio_create_context_from_uri("ip:can-link.local");
    defer iio.iio_context_destroy(context);
    if(context) |ctx| {
        const info = iio.iio_context_get_description(ctx);
        std.log.info("Connected to: {s}", .{info});

        const num_devices = iio.iio_context_get_devices_count(ctx);
        std.log.info("Counted {d} devices", .{num_devices});

        var devices = try allocator.alloc(Device, num_devices);
        // errdefer for(devices[0..num_devices]) |device| {
        //     allocator.destroy(device);
        // };

        for(devices) |*device, i| {
            device.data = iio.iio_context_get_device(ctx, @intCast(c_uint, i));
            const name = iio.iio_device_get_name(device.data);
            const nameSpan = std.mem.span(name);
            device.name = try allocator.dupe(u8, nameSpan);
        }

        for(devices) |*device, i| {
            std.log.info("device[{d}] {s}", .{i, device.name});
        }

        const chn = iio.iio_device_get_channel(devices[1].data, 1);
        var buf: [4]u8 = undefined;
        while(true) {
            _ = iio.iio_channel_attr_read(chn, "raw", &buf,4);
            std.log.info("valeu={d}", .{buf});
        }
    }
}
