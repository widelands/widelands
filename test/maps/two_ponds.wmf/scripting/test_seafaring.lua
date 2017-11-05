run(function()
   -- The map in its initial state has 2 unconnected port spaces, so it should not allow seafaring
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)

   -- Now add a connecting port space - we should have seafaring then
   map:set_port_space(0, 2, true)
   assert_equal(3, map.number_of_port_spaces)
   assert_equal(true, map.allows_seafaring)

   map:set_port_space(0, 2, false)
   assert_equal(2, map.number_of_port_spaces)
   assert_equal(false, map.allows_seafaring)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
