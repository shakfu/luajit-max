-- simple_luajit.lua
-- Simple example for luajit external
-- Demonstrates basic bang and int handling

external = {
    inlets = 1,
    outlets = 1
}

function external.bang()
    api.post("Bang received!")
    _outlets[1]:int(42)
end

function external.int(n)
    api.post("Received int: " .. n)
    _outlets[1]:int(n * 2)
end

function external.float(f)
    api.post("Received float: " .. f)
    _outlets[1]:float(f * 0.5)
end
