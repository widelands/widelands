run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   sleep(1000)
   start_building_farm()
   port1:set_wares{
      blackwood = 1,
   }
   sleep(10000)

   port2:remove()
   sleep(20000)
   assert_equal(port1:get_wares("blackwood"), 1)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
