-- Copyright (C) 2014 Anton Jouline (juce)


local format = string.format
local match = string.match
local tcp = ngx.socket.tcp
local tonumber = tonumber


local shell = {
    _VERSION = '0.01'
}

local sockproc_socket = "unix:/tmp/shell.sock"


function shell.execute(cmd, args)
    local timeout = args and args.timeout
    local input_data = args and args.data or ""
    local sock = tcp()
    local ok, err = sock:connect(sockproc_socket)
    if ok then
        sock:settimeout(timeout or 15000)
        sock:send(cmd .. "\r\n")
        sock:send(format("%d\r\n", #input_data))
        sock:send(input_data)

        -- status code
        local data, err, partial = sock:receive('*l')
        if err then
            return -1, nil, err
        end
        local code = match(data,"status:([-%d]+)") or -1

        -- output stream
        data, err, partial = sock:receive('*l')
        if err then
            return -1, nil, err
        end
        local n = tonumber(data) or 0
        local out_bytes = n > 0 and sock:receive(n) or nil

        -- error stream
        data, err, partial = sock:receive('*l')
        if err then
            return -1, nil, err
        end
        n = tonumber(data) or 0
        local err_bytes = n > 0 and sock:receive(n) or nil
        sock:close()

        return tonumber(code), out_bytes, err_bytes
    end
    return -2, nil, err
end


return shell

