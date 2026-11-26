-- The login of the web interface (off by default, switched on the settings page).
--
-- /etc/redpitaya/web_auth.json (rootfs, mode 600, survives an ecosystem update):
--   {"enabled": true, "hash": "pbkdf2-sha1$<iterations>$<salt hex>$<hash hex>"}
-- The login page is /auth.html.
-- Sessions: random tokens in the shared dictionary rp_auth (nginx.conf), sliding TTL; the cookie
-- rp_session is HttpOnly and SameSite=Strict. A restart of nginx ends every session.
-- While the login is on, the cookie rp_ws_proxy=1 tells the pages to reach the websocket servers
-- through nginx (/wsc, /wsm, /wsu, /wsd): their own ports are closed then (main_menu
-- scripts/settings.sh web-auth-firewall).
--
-- access(): the access phase of every location of server_common.conf.
-- login(), logout(), set(): the locations /auth_login, /auth_logout, /set_web_auth.

local bit = require "bit"

local M = {}

local CONF = "/etc/redpitaya/web_auth.json"
local SETTINGS = "/opt/redpitaya/www/apps/main_menu/scripts/settings.sh"
local TTL = 7 * 24 * 3600
local ITERATIONS = 20000
local dict = ngx.shared.rp_auth

-- reachable without a session: the login page and what it loads
local OPEN = {
    ["/auth.html"] = true,
    ["/auth_login"] = true,
    ["/auth_logout"] = true,
    ["/favicon.ico"] = true,
    ["/assets/images/rp_flame.png"] = true,
}

local function hex(s)
    return (s:gsub(".", function(c) return string.format("%02x", c:byte()) end))
end

local function random_bytes(n)
    local f = io.open("/dev/urandom", "rb")
    if not f then
        return nil
    end
    local b = f:read(n)
    f:close()
    return b
end

-- PBKDF2-HMAC-SHA1, one block (20 bytes); the sum is kept as numbers, no string per iteration
local function pbkdf2(password, salt, iterations)
    local bxor = bit.bxor
    local u = ngx.hmac_sha1(password, salt .. "\0\0\0\1")
    local t = { u:byte(1, 20) }
    for _ = 2, iterations do
        u = ngx.hmac_sha1(password, u)
        for i = 1, 20 do
            t[i] = bxor(t[i], u:byte(i))
        end
    end
    return string.char(unpack(t))
end

local function make_hash(password)
    local salt = random_bytes(16)
    if not salt then
        return nil
    end
    return "pbkdf2-sha1$" .. ITERATIONS .. "$" .. hex(salt) .. "$" .. hex(pbkdf2(password, salt, ITERATIONS))
end

local function check_hash(password, stored)
    local iter, salt_hex, hash_hex = (stored or ""):match("^pbkdf2%-sha1%$(%d+)%$(%x+)%$(%x+)$")
    if not iter then
        return false
    end
    local salt = salt_hex:gsub("%x%x", function(h) return string.char(tonumber(h, 16)) end)
    local got = hex(pbkdf2(password, salt, tonumber(iter)))
    -- compare every character: no early exit on the first difference
    if #got ~= #hash_hex then
        return false
    end
    local diff = 0
    for i = 1, #got do
        diff = bit.bor(diff, bit.bxor(got:byte(i), hash_hex:byte(i)))
    end
    return diff == 0
end

local function read_conf()
    local f = io.open(CONF, "r")
    if not f then
        return {}
    end
    local ok, c = pcall(cjson.decode, f:read("*a") or "")
    f:close()
    if not ok or type(c) ~= "table" then
        return {}
    end
    return c
end

local function write_conf(c)
    local tmp = CONF .. ".new"
    os.execute("mkdir -p /etc/redpitaya")
    local f = io.open(tmp, "w")
    if not f then
        return false
    end
    f:write(cjson.encode(c))
    f:close()
    os.execute("chmod 600 " .. tmp)
    return os.rename(tmp, CONF)
end

-- The state is read from the file again every 2 s: the dictionary survives a reload of nginx, and
-- a file removed over SSH (a forgotten password) must take effect without a restart.
local STATE_TTL = 2

local function enabled()
    local e = dict:get("enabled")
    if e == nil then
        local c = read_conf()
        e = (c.enabled == true and type(c.hash) == "string") and 1 or 0
        dict:set("enabled", e, STATE_TTL)
    end
    return e == 1
end

local function cookies(token)
    local secure = ngx.var.scheme == "https" and "; Secure" or ""
    if token then
        ngx.header["Set-Cookie"] = {
            "rp_session=" .. token .. "; Path=/; HttpOnly; SameSite=Strict; Max-Age=" .. TTL .. secure,
            "rp_ws_proxy=1; Path=/; SameSite=Strict; Max-Age=" .. TTL .. secure,
        }
    else
        ngx.header["Set-Cookie"] = {
            "rp_session=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0" .. secure,
            "rp_ws_proxy=; Path=/; SameSite=Strict; Max-Age=0" .. secure,
        }
    end
end

local function new_session()
    local b = random_bytes(32)
    if not b then
        return nil
    end
    local token = hex(b)
    dict:set("s:" .. token, 1, TTL)
    cookies(token)
    return token
end

local function valid_session()
    local token = ngx.var.cookie_rp_session
    if token and token:match("^%x+$") and dict:get("s:" .. token) then
        dict:set("s:" .. token, 1, TTL)
        return true
    end
    return false
end

local function reply(status, obj)
    ngx.status = status
    ngx.header["Content-Type"] = "application/json"
    ngx.say(cjson.encode(obj))
    return ngx.exit(status)
end

local function json_body()
    local ctype = ngx.var.content_type or ""
    if ngx.req.get_method() ~= "POST" or not ctype:find("application/json", 1, true) then
        return nil
    end
    ngx.req.read_body()
    local ok, req = pcall(cjson.decode, ngx.req.get_body_data() or "")
    if not ok or type(req) ~= "table" then
        return nil
    end
    return req
end

function M.access()
    if not enabled() or OPEN[ngx.var.uri] or valid_session() then
        return
    end
    local accept = ngx.var.http_accept or ""
    if ngx.req.get_method() == "GET" and accept:find("text/html", 1, true) then
        return ngx.redirect("/auth.html?next=" .. ngx.escape_uri(ngx.var.request_uri))
    end
    return ngx.exit(ngx.HTTP_UNAUTHORIZED)
end

function M.login()
    local req = json_body()
    if not req or type(req.password) ~= "string" then
        return reply(ngx.HTTP_BAD_REQUEST, { ok = false, error = "wrong request" })
    end
    local c = read_conf()
    if not enabled() then
        return reply(ngx.HTTP_OK, { ok = true })
    end
    -- One check at a time for the whole server: a check takes about 0.8 s of CPU (PBKDF2), so
    -- parallel requests could load the board and guess faster. The lock ends by itself (5 s).
    if not dict:add("login_busy", 1, 5) then
        return reply(429, { ok = false, error = "Another login is being checked, try again" })
    end
    local good = check_hash(req.password, c.hash)
    if not good then
        -- slows a guessing script down; the lock is held through the pause
        ngx.sleep(1)
        dict:delete("login_busy")
        ngx.log(ngx.WARN, "web login failed from ", ngx.var.remote_addr)
        return reply(ngx.HTTP_UNAUTHORIZED, { ok = false, error = "Wrong password" })
    end
    dict:delete("login_busy")
    if not new_session() then
        return reply(ngx.HTTP_INTERNAL_SERVER_ERROR, { ok = false, error = "no random source" })
    end
    return reply(ngx.HTTP_OK, { ok = true })
end

function M.logout()
    local token = ngx.var.cookie_rp_session
    if token and token:match("^%x+$") then
        dict:delete("s:" .. token)
    end
    cookies(nil)
    return ngx.redirect("/auth.html")
end

-- POST {"password": "<new>"} and/or {"enabled": true|false}; behind the login itself when it is on.
-- A new password ends the other sessions; the caller gets a session whenever the login stays on.
function M.set()
    local req = json_body()
    if not req then
        return reply(ngx.HTTP_BAD_REQUEST, { ok = false, error = "the request must be a POST of application/json" })
    end
    local c = read_conf()
    if req.password ~= nil then
        local p = req.password
        if type(p) ~= "string" or #p < 1 or #p > 128 or p:find("[%c]") then
            return reply(ngx.HTTP_BAD_REQUEST, { ok = false, error = "The password must be 1 to 128 characters long, without control characters" })
        end
        c.hash = make_hash(p)
        if not c.hash then
            return reply(ngx.HTTP_INTERNAL_SERVER_ERROR, { ok = false, error = "no random source" })
        end
        dict:flush_all()
    end
    if req.enabled ~= nil then
        if type(req.enabled) ~= "boolean" then
            return reply(ngx.HTTP_BAD_REQUEST, { ok = false, error = "wrong request" })
        end
        if req.enabled and type(c.hash) ~= "string" then
            return reply(ngx.HTTP_BAD_REQUEST, { ok = false, error = "Set a password first" })
        end
        c.enabled = req.enabled
    end
    if not write_conf(c) then
        return reply(ngx.HTTP_INTERNAL_SERVER_ERROR, { ok = false, error = "The settings were not written" })
    end
    local on = c.enabled == true and type(c.hash) == "string"
    dict:set("enabled", on and 1 or 0, STATE_TTL)
    os.execute(SETTINGS .. " web-auth-firewall " .. (on and "on" or "off") .. " >/dev/null 2>&1")
    if on then
        new_session()
    else
        cookies(nil)
    end
    ngx.log(ngx.WARN, "web login settings changed from ", ngx.var.remote_addr, ": ", on and "on" or "off",
        req.password ~= nil and ", new password" or "")
    return reply(ngx.HTTP_OK, { ok = true, enabled = on })
end

return M
