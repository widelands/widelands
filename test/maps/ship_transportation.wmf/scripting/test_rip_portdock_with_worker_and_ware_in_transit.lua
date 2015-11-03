run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_southern_port()
   create_northern_port()
   
   --removing builder from northern port
    northern_port():set_workers{
      builder = 0,
   }  

   assert_equal(northern_port():get_workers("builder"), 0)

   start_building_farm()
   southern_port():set_workers{
      builder = 1,
   }
   southern_port():set_wares{
      blackwood = 1,
   }

   assert_equal(southern_port():get_workers("builder"), 1)
   assert_equal(southern_port():get_wares("blackwood"), 1)
   
   while ship:get_workers() == 0 or ship:get_wares() == 0  do
      sleep(500) 
   end

   local flag_oversea = northern_port().flag   

 	stable_save("restored_port")
   
   -- remove the portdock while the blackwood is in transit.
   north_port_portdock=northern_port().portdock
   assert(north_port_portdock)
   north_port_portdock:remove()
   
   sleep(5000)

   assert_equal(p1:get_workers("builder"), 1)
   assert_equal(p1:get_wares("blackwood"), 1)
   assert_equal(ship.debug_economy, southern_port().debug_economy)
   assert_equal(ship.debug_economy, flag_oversea.debug_economy)

   sleep(5000)
   
   --just wait till everything is gone to northern port
   while ship:get_workers() > 0 or ship:get_wares() > 0 or
    southern_port():get_workers("builder") > 0 or  southern_port():get_wares("blackwood") > 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
