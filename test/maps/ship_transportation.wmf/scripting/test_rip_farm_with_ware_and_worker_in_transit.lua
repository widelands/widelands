run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   start_building_farm()
   port1():set_wares{
      blackwood = 1,
   }
   port1():set_workers{
      builder = 1,
   }

   assert_equal(p1:get_wares("blackwood"), 1)
   assert_equal(p1:get_workers("builder"), 1)

   while not (ship:get_wares() == 1 and ship:get_workers() == 1) do
      sleep(100)
   end

   -- kill the farm while the blackwood is in transit.
   farm():remove()

   while not (port2():get_wares("blackwood") == 1
      and port2():get_workers("builder") == 1) do
      sleep(100)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
