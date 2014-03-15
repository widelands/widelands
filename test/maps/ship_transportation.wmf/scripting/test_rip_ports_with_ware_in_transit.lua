run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   start_building_farm()
   port1():set_wares{
      blackwood = 1,
   }

   assert_equal(p1:get_wares("blackwood"), 1)
   while ship:get_wares() == 0 do sleep(2000) end
   assert_equal(p1:get_wares("blackwood"), 1)

   -- kill the port while the blackwood is in transit.
   local flag_oversea = port2().flag
   port2():remove()
   assert_equal(p1:get_wares("blackwood"), 1)
   sleep(1000)

   assert_equal(p1:get_wares("blackwood"), 1)
   assert_equal(ship.debug_economy, port1().debug_economy)
   assert_not_equal(ship.debug_economy, flag_oversea.debug_economy)

   -- now kill the first port too.
   port1():remove()
   sleep(1000)

   stable_save("no_ports")

   -- There are no more ports, therefore also no fleet. The wares on the ship
   -- are not accessible and should therefore not show up in the stock anymore.
   -- One could also argue that the ship is an economy of its own now and the
   -- wares should still appear in stock. Such a corner case that it seems not
   -- to matter.
   assert_equal(p1:get_wares("blackwood"), 0)

   -- Give the ship enough time to react on the new situation.
   sleep(30000)

   -- Recreate the first port.
   create_first_port()
   sleep(1000)
   assert_equal(p1:get_wares("blackwood"), 1)

   -- Wait (and hope) that the ship will eventually return the blackwood.
   while port1():get_wares("blackwood") == 0 do
      sleep(5000)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
