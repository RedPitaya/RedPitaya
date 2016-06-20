local shell = require("shell")

local status, out, err = shell.execute("ip addr list eth0 > /tmp/eth0_status.txt")


local f = io.open("/tmp/eth0_status.txt", "r")
local content = f:read("*all")
f:close()
ngx.say(content)
