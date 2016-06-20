local shell = require("shell")

local status, out, err = shell.execute("dhclient -r eth0")
local status, out, err = shell.execute("dhclient eth0")
