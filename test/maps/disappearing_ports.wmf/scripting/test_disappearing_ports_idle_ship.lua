run(function()
   sleep(100)

   create_first_port()
   create_second_port()

   sleep(2000)

   port1:remove()
   port2:remove()

   sleep(2000)

   create_first_port()

   start_building_farm()

   create_second_port()

   game.desired_speed = 100 * 1000
   while not is_farm_done() do
      sleep(2154)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
