include "scripting/lunit.lua"

-- ============
-- Basic tests
-- ============
use_test = lunit.TestCase("Use test")
function use_test:test_use_invalid_name()
   assert_error("not existant", function() include("thisdoesntexist") end)
end
function use_test:test_use_invalid_name_in_map()
   assert_error("not existant", function() include("map:blubi.lua") end)
end
