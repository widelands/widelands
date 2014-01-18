run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   local wh = p1:place_building("warehouse", map:get_field(13, 16), false, false)
   connected_road(p1, wh.flag, "r,r,r", true)
   create_second_port()

   start_building_farm()
   port1():set_wares{
      blackwood = 1,
   }

   -- Wait till the ware is in the portdock.
   while port1():get_wares("blackwood") == 1 do
      sleep(200)
   end
   assert_equal(p1:get_wares("blackwood"), 1)
   sleep(8000)


   port1():remove()

   sleep(100)
   assert_equal(p1:get_wares("blackwood"), 0)
   stable_save("port1_just_removed")

   sleep(5000)

   -- Create port again.
   create_first_port()
   wh:set_wares("blackwood", 1)

   sleep(200)
   assert_equal(p1:get_wares("blackwood"), 1)

   while ship:get_wares() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
