run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   local wh = p1:place_building("warehouse", map:get_field(13, 16), false, false)
   connected_road(p1, wh.flag, "r,r,r", true)
   create_second_port()

   start_building_farm()
   port1():set_workers{
      builder = 1,
   }

   -- Wait till the worker transfers to the portdock.
   while port1():get_workers("builder") == 1 do
      sleep(200)
   end
   sleep(3000)

   port1():remove()
   sleep(100)
   stable_save("port1_just_removed")

   -- Wait till the worker runs to the warehouse.
   while wh:get_workers("builder") == 0 do
      sleep(200)
   end

   -- Create port again.
   create_first_port()

   while ship:get_workers() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
