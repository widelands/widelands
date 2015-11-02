run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_southern_port()
   create_northern_port()

   start_building_farm()
   southern_port():set_workers{
      builder = 1,
   }

   assert_equal(p1:get_workers("builder"), 1)
   while ship:get_workers() == 0 do sleep(2000) end
   assert_equal(p1:get_workers("builder"), 1)

   -- kill the port while the builder is in transit.
   local flag_oversea = northern_port().flag
   northern_port():remove()
   assert_equal(p1:get_workers("builder"), 1)
   sleep(1000)

   assert_equal(p1:get_workers("builder"), 1)
   assert_equal(ship.debug_economy, southern_port().debug_economy)
   assert_not_equal(ship.debug_economy, flag_oversea.debug_economy)

   -- now kill the first port too.
   southern_port():remove()
   sleep(1000)

   stable_save("no_ports")

   -- There are no more ports, therefore also no fleet. The workers on the ship
   -- are not accessible and should therefore not show up in the stock anymore.
   -- One could also argue that the ship is an economy of its own now and the
   -- workers should still appear in stock. Such a corner case that it seems not
   -- to matter.
   assert_equal(p1:get_workers("builder"), 0)

   -- Give the ship enough time to react on the new situation.
   sleep(30000)

   -- Recreate the first port.
   create_southern_port()
   sleep(1000)
   assert_equal(p1:get_workers("builder"), 1)

   -- Wait (and hope) that the ship will eventually return the builder.
   while southern_port():get_workers("builder") == 0 do
      sleep(5000)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
