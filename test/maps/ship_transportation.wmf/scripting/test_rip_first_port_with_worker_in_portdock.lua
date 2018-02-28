run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_southern_port()
   local wh = p1:place_building("barbarians_warehouse", map:get_field(13, 16), false, false)
   connected_road(p1, wh.flag, "r,r,r", true)
   create_northern_port()

   start_building_farm()
   southern_port():set_workers{
      barbarians_builder = 1,
   }

   -- Wait till the worker transfers to the portdock.
   while southern_port():get_workers("barbarians_builder") == 1 do
      sleep(200)
   end
   sleep(3000)

   southern_port():remove()
   sleep(100)
   stable_save(game, "port1_just_removed")

   -- Wait till the worker runs to the warehouse.
   while wh:get_workers("barbarians_builder") == 0 do
      sleep(200)
   end

   -- Create port again.
   create_southern_port()

   while ship:get_workers() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
