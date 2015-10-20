run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   local wh = p1:place_building("warehouse", map:get_field(13, 16), false, false)
   connected_road(p1, wh.flag, "r,r,r", true)
   create_northern_port()

   start_building_farm()
   southern_port():set_wares{
      blackwood = 1,
   }

   -- Wait till the ware is in the portdock.
   while southern_port():get_wares("blackwood") == 1 do
      sleep(200)
   end
   assert_equal(p1:get_wares("blackwood"), 1)
   sleep(8000)


   southern_port():remove()

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
