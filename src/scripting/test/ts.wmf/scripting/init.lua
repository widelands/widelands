function include(s) 
   use("map", s)
end

include "lunit"
lunit.import "assertions"

-- ============
-- Basic tests 
-- ============
use_test = lunit.TestCase("Use test")
function use_test:test_use_invalid_name()
   assert_error("not existant", function() use("map", "thisdoesntexist") end)
end
function use_test:test_use_invalid_ns()
   assert_error("not existant", function() use("thisdoesntexist", "init") end)
end

include "test_field"
include "test_immovables"
include "test_player"

rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the game
   wl.debug.exit()
else
   wl.debug.set_see_all(1) -- Reveal everything, easier for debugging
end

