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

-- =============================
-- Test random number generation 
-- =============================
-- Widelands overwrites luas random generator with it's own so that 
-- games stay in sync over networks and can be replayed
random_test = lunit.TestCase("math.random test")
function random_test:test_create_floats()
   local f = {}
   for i=1,1000 do f[i] = math.random() end
   function do_test(t)
      for i=1,1000 do if t[i] >= 1 then return false end end
      return true
   end
   assert_true(do_test(f))
end
function random_test:test_create_integer()
   local f = {}
   for i=1,1000 do f[i] = math.random(20) end
   function do_test(t)
      for i=1,1000 do
         if t[i] < 1 or t[i] > 20 then return false end
      end
      return true
   end
   assert_true(do_test(f))
end
function random_test:test_create_integer_range()
   local f = {}
   for i=1,1000 do f[i] = math.random(10,20) end
   function do_test(t)
      for i=1,1000 do
         if t[i] < 10 or t[i] > 20 then return false end
      end
      return true
   end
   assert_true(do_test(f))
end

include "test_map"
include "test_field"
include "test_immovables"
include "test_player"
include "test_table"

rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the game
   wl.debug.exit()
else
   wl.debug.set_see_all(1) -- Reveal everything, easier for debugging
end

