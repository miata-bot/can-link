pub const lua = @cImport({
    @cDefine("LUA_COMPAT_MODULE", {});
    @cInclude("lua.h");
    @cInclude("lualib.h");
    @cInclude("lauxlib.h");
});
