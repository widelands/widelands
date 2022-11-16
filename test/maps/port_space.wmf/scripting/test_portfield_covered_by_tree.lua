run(function()
   --  This places some trees which do not make the portspace unusable
   map:place_immovable("alder_summer_old", map:get_field(7,10))
   map:place_immovable("alder_summer_sapling", map:get_field(7,11))
   map:place_immovable("alder_summer_pole", map:get_field(6,10))
   map:place_immovable("alder_summer_mature", map:get_field(7,9))
   local ship = create_infrastructure(59, 10)
   game.desired_speed = 1000
   wait_for_message("Port Space")
   -- test port lost detection with a rock
   map:place_immovable("greenland_rocks1", map:get_field(5,9))
   wait_for_message("Port Space Lost!")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
