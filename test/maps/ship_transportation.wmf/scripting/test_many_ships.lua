 
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- used to test naming of ships

   i = 0
   while i < 10 do
        p1:place_ship(map:get_field(10, 10))
        i = i + 1
        sleep(2000)                                                                                                                    
        end
 
  stable_save("10_ships")
   
   i = 0
   while i < 10 do
        p1:place_ship(map:get_field(10, 10))
        i = i + 1
        sleep(2000)
        end

  stable_save("20_ships")
  
    i = 0
   while i < 10 do
        p1:place_ship(map:get_field(10, 10))
        i = i + 1
        sleep(2000)
        end 
        sleep(20000)
   
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
