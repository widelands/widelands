run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_northern_port()

   start_building_farm()
   southern_port():set_wares{
      blackwood = 1,
   }
   
   sleep(1000)

   -- The ship should not yet have picked up the ware from the
   -- portdock.
   assert_equal(1, p1:get_wares("blackwood"))
   assert_equal(0, southern_port():get_wares("blackwood"))
   
   while ship:get_wares() == 0 do
   	  -- ship still on the way to the bottom port
      sleep(50)
   end

   --now it is loaded with something and port empty.. 
   sleep(2000)   
   assert_equal(0, southern_port():get_wares("blackwood"))
   -- ...and on the way to the north, so let remove the upper port
   northern_port():remove()
   sleep(100)

   stable_save("ware_in_portdock")

   --ship has to get to the place of former upper port and then return back to the bottom port
   sleep(30000)

   -- Ware should be back in port now.
   assert_equal(1, p1:get_wares("blackwood"))
   assert_equal(1, southern_port():get_wares("blackwood"))

   -- Create port again.
   create_northern_port()
	
   sleep (10000)
	
   while ship:get_wares() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
