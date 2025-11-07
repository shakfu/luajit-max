-- test_api.lua
-- Test script for the api module

-- Test console functions
if api then
    api.post("API module loaded successfully!")

    -- Test Symbol
    local sym1 = api.gensym("test")
    api.post("Created symbol: " .. tostring(sym1))
    api.post("Symbol name: " .. sym1:name())

    local sym2 = api.Symbol("test")
    api.post("Symbol comparison: " .. tostring(sym1 == sym2))
    api.post("String comparison: " .. tostring(sym1 == "test"))

    -- Test Atom
    local atom_int = api.Atom(42)
    api.post("Integer atom: " .. tostring(atom_int))
    api.post("Atom type: " .. atom_int:type())
    api.post("Atom value: " .. tostring(atom_int:getlong()))

    local atom_float = api.Atom(3.14159)
    api.post("Float atom: " .. tostring(atom_float))
    api.post("Float value: " .. tostring(atom_float:getfloat()))

    local atom_sym = api.Atom("hello")
    api.post("Symbol atom: " .. tostring(atom_sym))
    api.post("Is symbol: " .. tostring(atom_sym:is_symbol()))

    -- Test parse
    local atoms = api.parse("foo 123 4.56 bar")
    api.post("Parsed " .. #atoms .. " atoms")
    for i, atom in ipairs(atoms) do
        api.post("  [" .. i .. "] " .. tostring(atom))
    end

    -- Test atom_gettext
    local text = api.atom_gettext(atoms)
    api.post("Reconstructed text: " .. text)

    api.post("API tests completed!")
else
    print("ERROR: api module not loaded!")
end

-- DSP function (passthrough)
function process(input, prev, n, param)
    return input
end
