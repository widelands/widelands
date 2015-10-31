run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   --removing builder from port2
    port2():set_workers{
      barbarians_builder = 0,
   }

   assert_equal(port2():get_workers("barbarians_builder"), 0)

   start_building_farm()
   port1():set_workers{
      barbarians_builder = 1,
   }
   port1():set_wares{
      blackwood = 1,
   }

   assert_equal(port1():get_workers("barbarians_builder"), 1)
   assert_equal(port1():get_wares("blackwood"), 1)

   while ship:get_workers() == 0 or ship:get_wares() == 0  do
      sleep(500)
   end

   local flag_oversea = port2().flag

   stable_save("restored_port")

   -- remove the portdock while the blackwood is in transit.
   port2_portdock=port2().portdock
   assert(port2_portdock)
   port2_portdock:remove()

   sleep(5000)

   assert_equal(p1:get_workers("barbarians_builder"), 1)
   assert_equal(p1:get_wares("blackwood"), 1)
   assert_equal(ship.debug_economy, port1().debug_economy)
   assert_equal(ship.debug_economy, flag_oversea.debug_economy)

   sleep(5000)

   --just wait till everything is gone to port2
   while ship:get_workers() > 0 or ship:get_wares() > 0 or
    port1():get_workers("barbarians_builder") > 0 or  port1():get_wares("blackwood") > 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
