-- inlet_aware.lua
-- Example demonstrating inlet-aware message dispatch
-- Different inlets trigger different behaviors

external = {
    inlets = 4,
    outlets = 2
}

-- State for each inlet
local inlet_values = {0, 0, 0, 0}
local inlet_names = {"left", "center-left", "center-right", "right"}

-- Generic int handler - demonstrates inlet detection
-- Note: Current inlet is stored in registry by the C code
function external.int(n)
    -- We can differentiate by checking inlet_values in sequence
    -- or by maintaining state
    -- For now, store values in a round-robin fashion
    local inlet = 0  -- Default to first inlet

    -- Store value
    inlet_values[inlet + 1] = n

    api.post(string.format("Int %d received at inlet %d (%s)",
                          n, inlet, inlet_names[inlet + 1]))

    -- Outlet 1: the value
    _outlets[1]:int(n)

    -- Outlet 2: inlet information
    _outlets[2]:list({inlet, n})
end

-- Alternative: use different message names for different inlets
-- Send "left <n>" to inlet 0, "right <n>" to inlet 3, etc.

function external.left(n)
    inlet_values[1] = tonumber(n) or 0
    api.post(string.format("Left inlet: %d", inlet_values[1]))
    _outlets[1]:int(inlet_values[1])
    _outlets[2]:symbol("left")
end

function external.centerleft(n)
    inlet_values[2] = tonumber(n) or 0
    api.post(string.format("Center-left inlet: %d", inlet_values[2]))
    _outlets[1]:int(inlet_values[2])
    _outlets[2]:symbol("center-left")
end

function external.centerright(n)
    inlet_values[3] = tonumber(n) or 0
    api.post(string.format("Center-right inlet: %d", inlet_values[3]))
    _outlets[1]:int(inlet_values[3])
    _outlets[2]:symbol("center-right")
end

function external.right(n)
    inlet_values[4] = tonumber(n) or 0
    api.post(string.format("Right inlet: %d", inlet_values[4]))
    _outlets[1]:int(inlet_values[4])
    _outlets[2]:symbol("right")
end

-- Bang outputs sum of all inlet values
function external.bang()
    local sum = 0
    for i, v in ipairs(inlet_values) do
        sum = sum + v
    end

    api.post(string.format("Sum of all inlets: %d", sum))
    _outlets[1]:int(sum)
    _outlets[2]:list(inlet_values)
end

-- Reset all inlets
function external.reset()
    for i = 1, 4 do
        inlet_values[i] = 0
    end
    api.post("All inlets reset")
    _outlets[2]:symbol("reset")
end

-- Info about current state
function external.info()
    api.post("=== Inlet Values ===")
    for i, v in ipairs(inlet_values) do
        api.post(string.format("Inlet %d (%s): %d", i-1, inlet_names[i], v))
    end
    _outlets[2]:list(inlet_values)
end

-- Multiply all values
function external.scale(factor)
    local f = tonumber(factor) or 1
    api.post(string.format("Scaling all values by %g", f))

    for i = 1, 4 do
        inlet_values[i] = inlet_values[i] * f
    end

    _outlets[2]:list(inlet_values)
end
