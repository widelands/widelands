run(function()
   -- The map in its initial state has 2 unconnected port spaces, so it should
   -- not allow seafaring. One of the port spaces has trees on top of it.
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(false, map.allows_seafaring)

   -- Now try to add a port space on a medium buildcap, it should fail
   assert_equal(false, map:set_port_space(11, 9, true))
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(false, map.allows_seafaring)

   -- Now try to add a port space away from water, it should fail
   assert_equal(false, map:set_port_space(18, 9, true))
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(false, map.allows_seafaring)

   -- Now add a connecting port space - it should succeed and we should have seafaring then
   assert_equal(true, map:set_port_space(0, 2, true))
   assert_equal(3, map.number_of_port_spaces)
   assert_equal(true, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(true, map.allows_seafaring)

   stable_save(game, "port_spaces", 1000)
   assert_equal(3, map.number_of_port_spaces)
   assert_equal(true, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(true, map.allows_seafaring)

   local port_spaces = map.port_spaces
   assert_equal(port_spaces[1]["x"], 0)
   assert_equal(port_spaces[1]["y"], 2)
   assert_equal(port_spaces[2]["x"], 7)
   assert_equal(port_spaces[2]["y"], 2)
   assert_equal(port_spaces[3]["x"], 12)
   assert_equal(port_spaces[3]["y"], 24)

  -- Remove the port space again
   assert_equal(true, map:set_port_space(0, 2, false))
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)
   map:recalculate_seafaring()
   assert_equal(false, map.allows_seafaring)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
