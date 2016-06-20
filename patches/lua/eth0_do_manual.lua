local shell = require("shell")


local args = ngx.req.get_uri_args()
local ipaddr = args.ipaddr or 0
local brdaddr = args.brdaddr or 0
local netmask = args.netmask or 0

local status, out, err = shell.execute("dhclient -r eth0")
local status, out, err = shell.execute("ip addr add "..ipaddr.."/"..netmask.." dev eth0")
local status, out, err = shell.execute("ip addr add brd "..brdaddr.." dev eth0")
