-- math_ops.lua
-- Math operations example with custom messages
-- Demonstrates custom message handlers and list output

external = {
    inlets = 2,
    outlets = 2
}

local value1 = 0
local value2 = 0

function external.bang()
    -- Output current values
    _outlets[1]:float(value1)
    _outlets[2]:float(value2)
end

function external.int(n)
    value1 = n
    api.post("Value1 set to: " .. value1)
end

function external.float(f)
    value1 = f
    api.post("Value1 set to: " .. value1)
end

-- Custom message: add
function external.add()
    local result = value1 + value2
    api.post(string.format("%g + %g = %g", value1, value2, result))
    _outlets[1]:float(result)
end

-- Custom message: subtract
function external.subtract()
    local result = value1 - value2
    api.post(string.format("%g - %g = %g", value1, value2, result))
    _outlets[1]:float(result)
end

-- Custom message: multiply
function external.multiply()
    local result = value1 * value2
    api.post(string.format("%g * %g = %g", value1, value2, result))
    _outlets[1]:float(result)
end

-- Custom message: divide
function external.divide()
    if value2 ~= 0 then
        local result = value1 / value2
        api.post(string.format("%g / %g = %g", value1, value2, result))
        _outlets[1]:float(result)
    else
        api.error("Cannot divide by zero!")
    end
end

-- Custom message: power
function external.power()
    local result = value1 ^ value2
    api.post(string.format("%g ^ %g = %g", value1, value2, result))
    _outlets[1]:float(result)
end

-- Custom message: set - takes two arguments
function external.setvalues(v1, v2)
    value1 = tonumber(v1) or 0
    value2 = tonumber(v2) or 0
    api.post(string.format("Set values: %g, %g", value1, value2))
end

-- Output all results as a list
function external.all()
    local add = value1 + value2
    local sub = value1 - value2
    local mul = value1 * value2
    local div = value2 ~= 0 and (value1 / value2) or 0

    -- Output list
    _outlets[1]:list({add, sub, mul, div})
end
