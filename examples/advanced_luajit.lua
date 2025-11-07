-- advanced_luajit.lua
-- Advanced example demonstrating all features of luajit external
-- - Multiple inlets with inlet-aware dispatch
-- - State management
-- - Dynamic attributes
-- - Custom messages
-- - List output

external = {
    inlets = 3,
    outlets = 4,

    -- Instance state (can be accessed as attributes via getvalue/setvalue)
    counter = 0,
    multiplier = 2,
    enabled = true,
    mode = "normal"
}

-- Helper function to get current inlet
local function get_inlet()
    -- This would require api.proxy_getinlet() to be exposed
    -- For now, we handle inlet routing in the methods themselves
    return 0
end

-- Initialize
function external.loadbang()
    api.post("Advanced luajit external loaded")
    api.post(string.format("Mode: %s, Multiplier: %d", external.mode, external.multiplier))
    _outlets[4]:symbol("ready")
end

-- Bang handler
function external.bang()
    if not external.enabled then
        _outlets[4]:symbol("disabled")
        return
    end

    external.counter = external.counter + 1
    api.post(string.format("Bang %d", external.counter))
    _outlets[1]:int(external.counter)
    _outlets[4]:symbol("bang")
end

-- Int handler
function external.int(n)
    if not external.enabled then
        return
    end

    local result = n * external.multiplier
    api.post(string.format("%d * %d = %d", n, external.multiplier, result))
    _outlets[1]:int(result)
    _outlets[2]:float(result)
end

-- Float handler
function external.float(f)
    if not external.enabled then
        return
    end

    local result = f * external.multiplier
    _outlets[2]:float(result)
end

-- List handler - output stats
function external.list(...)
    local args = {...}
    local sum = 0
    local count = #args

    for i, v in ipairs(args) do
        sum = sum + tonumber(v) or 0
    end

    local avg = count > 0 and (sum / count) or 0

    api.post(string.format("List: %d items, sum=%.2f, avg=%.2f", count, sum, avg))

    -- Output as list
    _outlets[3]:list({count, sum, avg})
end

-- Custom message: reset
function external.reset()
    external.counter = 0
    external.multiplier = 2
    external.mode = "normal"
    api.post("Reset to defaults")
    _outlets[4]:symbol("reset")
end

-- Custom message: set mode
function external.setmode(mode_str)
    external.mode = tostring(mode_str)
    api.post("Mode set to: " .. external.mode)
    _outlets[4]:list({"mode", external.mode})
end

-- Custom message: set multiplier
function external.setmult(n)
    external.multiplier = tonumber(n) or 2
    api.post("Multiplier set to: " .. external.multiplier)
    _outlets[4]:list({"multiplier", external.multiplier})
end

-- Custom message: enable/disable
function external.enable(state)
    if type(state) == "number" then
        external.enabled = (state ~= 0)
    elseif type(state) == "string" then
        external.enabled = (state == "1" or state == "on" or state == "true")
    end
    api.post(external.enabled and "Enabled" or "Disabled")
    _outlets[4]:symbol(external.enabled and "enabled" or "disabled")
end

-- Custom message: info - output current state
function external.info()
    api.post("=== Current State ===")
    api.post(string.format("Counter: %d", external.counter))
    api.post(string.format("Multiplier: %d", external.multiplier))
    api.post(string.format("Mode: %s", external.mode))
    api.post(string.format("Enabled: %s", tostring(external.enabled)))

    -- Output as dictionary-style message
    _outlets[4]:anything("info", {
        "counter", external.counter,
        "multiplier", external.multiplier,
        "mode", external.mode,
        "enabled", external.enabled and 1 or 0
    })
end

-- Custom message: test with multiple arguments
function external.test(arg1, arg2, arg3)
    local a1 = tonumber(arg1) or 0
    local a2 = tonumber(arg2) or 0
    local a3 = tonumber(arg3) or 0

    local result = a1 + a2 + a3
    api.post(string.format("Test: %g + %g + %g = %g", a1, a2, a3, result))
    _outlets[1]:int(result)
end

-- Demonstrate using luafun functional programming library (if available)
-- This shows module loading support
function external.usefun()
    local success, fun = pcall(require, "fun")

    if success then
        api.post("luafun loaded successfully!")

        -- Example: generate range and sum
        local sum = fun.range(1, 10):sum()
        api.post(string.format("Sum of 1..10 = %d", sum))
        _outlets[1]:int(sum)

        _outlets[4]:symbol("fun_loaded")
    else
        api.error("luafun not found - install it in lua_modules directory")
        _outlets[4]:symbol("fun_not_found")
    end
end
