package.path = package.path..";/opt/redpitaya/www/conf/lua/?.lua"

local shell = require("shell")
--shell.execute("rm -fr /tmp/online.txt")
shell.execute("> /tmp/online.txt")
shell.execute("wget http://downloads.redpitaya.com/ -O /tmp/online.txt 2> /dev/null")

local fh = io.open("/tmp/online.txt", "r")
if not fh then
    ngx.status = 404
    return ngx.exit(ngx.HTTP_NOT_FOUND)
end
line = fh:read()
if line == nil then
    ngx.status = 404
    fh:close()
    return ngx.exit(ngx.HTTP_NOT_FOUND)
else
    fh:close()
    ngx.say("OK")
end
