include "test/scripting/stable_save.lua"

-- Test saveloading of input queues
run(function()

   local function assert_all_queues_are_full(building)
      local inputs = building:get_inputs("all")
      for name, amount in pairs(building.valid_inputs) do
         assert_equal(amount, inputs[name])
      end
   end

   local function assert_all_queues_are_empty(building)
      local inputs = building:get_inputs("all")
      for name, amount in pairs(building.valid_inputs) do
         assert_equal(0, inputs[name])
      end
   end

   sleep(1000)

   local b = p1:place_building("barbarians_barracks", map:get_field(25, 25), false, true)
   sleep(1000)

   -- Test start/stop and ensure that the building is stopped
   local building_was_stopped = b.is_stopped

   b:toggle_start_stop()
   sleep(1000)
   assert_equal(not building_was_stopped, b.is_stopped)

   b:toggle_start_stop()
   sleep(1000)
   assert_equal(building_was_stopped, b.is_stopped)

   if not b.is_stopped then
      b:toggle_start_stop()
      sleep(1000)
   end

   -- Now test filling the queues and workers
   assert_all_queues_are_empty(b)

   b:set_inputs(b.valid_inputs)
   b:set_workers(b.valid_workers)

   assert_all_queues_are_full(b)

   stable_save(game, "inputqueues", 50 * 1000)
   -- If we don't sleep long enough, this test can stall.
   sleep(10 * game.desired_speed)

   assert_all_queues_are_full(b)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
