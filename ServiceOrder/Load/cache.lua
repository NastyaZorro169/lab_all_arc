local frandom = io.open("/dev/urandom", "rb")
local d = frandom:read(4)
math.randomseed(d:byte(1) + (d:byte(2) * 256) + (d:byte(3) * 65536) + (d:byte(4) * 4294967296))

number =  math.random(1,10000)
request = function()
    headers = {}
    headers["Content-Type"] = "application/json"
    headers["Authorization"] = "Basic Q2h1Y2tfRHVuYmFyNDgxMkBhdGluay5jb206cHdkMTIz" -- JuliusChadwickJulius_Chadwick918@c2nyu.tech qwertyuiop
    body = ''
    return wrk.format("GET", "/user?id=".. tostring(number), headers, body)
end