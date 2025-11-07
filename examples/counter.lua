-- counter.lua
-- Counter example with multiple inlets and outlets
-- Left inlet: bang to output current count, int to set count
-- Right inlet: int to set step value
-- Outlets: count value, step value, status messages

external = {
    inlets = 2,
    outlets = 3
}

-- Instance state
local count = 0
local step = 1

function external.bang()
    api.post("Count: " .. count)
    _outlets[1]:int(count)
    _outlets[3]:symbol("bang")
    count = count + step
end

function external.int(n)
    -- Get current inlet number (0-indexed)
    -- Note: This is a placeholder - need to implement inlet detection
    -- For now, assume first inlet
    count = n
    api.post("Count set to: " .. count)
    _outlets[2]:int(step)
end

function external.float(f)
    count = math.floor(f)
    api.post("Count set to: " .. count)
end

function external.reset()
    count = 0
    step = 1
    api.post("Counter reset")
    _outlets[1]:int(count)
    _outlets[2]:int(step)
    _outlets[3]:symbol("reset")
end

function external.set(...)
    local args = {...}
    if #args >= 1 then
        count = args[1]
    end
    if #args >= 2 then
        step = args[2]
    end
    api.post(string.format("Set count=%d, step=%d", count, step))
    _outlets[1]:int(count)
    _outlets[2]:int(step)
end

function external.list(...)
    local args = {...}
    api.post("List received with " .. #args .. " elements:")
    for i, v in ipairs(args) do
        api.post("  [" .. i .. "] = " .. tostring(v))
    end
    if #args > 0 then
        _outlets[1]:int(#args)
    end
end
