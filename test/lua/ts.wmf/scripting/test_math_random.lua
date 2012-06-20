-- =============================
-- Test random number generation 
-- =============================
-- Widelands overwrites Luas random generator with it's own so that
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


