 
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- We will collect names of first 20 ships to confirm they are unique
   -- hopefully the tribe has more then 20 ship names available
   ships = {}

   i = 0
   while i < 20 do
        newship = p1:place_ship(map:get_field(10, 10))
        assert(newship)
        
        --make sure this name is not in used names yet
    	for _, this_ship_name in pairs(ships) do
   			assert_not_equal(this_ship_name, newship.shipname)
		end
        
        table.insert(ships, newship.shipname)

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
   
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
