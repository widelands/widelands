-- ==================================================
-- Tests for Economy that are only useful in the Game
-- ==================================================

economy_tests = lunit.TestCase("Economy test")
function economy_tests:test_instantiation_forbidden()
   assert_error("Cannot instantiate", function()
      wl.map.Economy()
   end)
end

function economy_tests:test_ware_target_quantity()

   -- Get the economy off a flag
   local sf = map:get_field(10, 10)
   local hq = player1:place_building("barbarians_headquarters", sf, false, true)
   local hq_flag = hq.flag
   local eco = hq_flag.economy

   -- Test illegal parameters
   assert_error("Nonexisting ware",function() eco:ware_target_quantity("foobar") end)
   assert_error("Quantity for nonexisting ware",function() eco:worker_target_quantity("foobar", 1) end)
   assert_error("Negative ware quantity",function() eco:set_ware_target_quantity("log", -1) end)

   -- Now set and confirm ware quantity
   quantity = eco:ware_target_quantity("log")
   quantity = quantity + 1
   eco:set_ware_target_quantity("log", quantity)
   assert_equal(quantity, eco:ware_target_quantity("log"))

   hq_flag:remove()
end

function economy_tests:test_worker_target_quantity()
   -- Get the economy off a flag
   local sf = map:get_field(10, 10)
   local hq = player1:place_building("barbarians_headquarters", sf, false, true)
   local hq_flag = hq.flag
   local eco = hq_flag.economy

   -- Test illegal parameters
   assert_error("Nonexisting worker",function() eco:worker_target_quantity("foobar") end)
   assert_error("Quantity for nonexisting worker",function() eco:worker_target_quantity("foobar", 1) end)
   assert_error("Negative worker quantity",function() eco:set_worker_target_quantity("barbarians_soldier", -1) end)

   -- Now set and confirm worker quantity
   quantity = eco:worker_target_quantity("barbarians_soldier")
   quantity = quantity + 1
   eco:set_worker_target_quantity("barbarians_soldier", quantity)
   assert_equal(quantity, eco:worker_target_quantity("barbarians_soldier"))
   hq_flag:remove()
end
