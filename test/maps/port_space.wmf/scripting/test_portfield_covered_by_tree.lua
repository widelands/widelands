run(function()
   --  This places some trees which do not make the portspace unusable
   map:place_immovable("alder_summer_old", map:get_field(7,10))
   map:place_immovable("alder_summer_sapling", map:get_field(7,11))
   map:place_immovable("alder_summer_pole", map:get_field(6,10))
   map:place_immovable("alder_summer_mature", map:get_field(7,9))

   local ship = create_infrastructure(59, 10)
   game.desired_speed = 1000
   print("Checking detection of port space covered by trees")
   wait_for_message("Port Space")

   -- test port lost detection with a rock
   print("Invalidating port space with a neighbouring rock")
   map:place_immovable("greenland_rocks1", map:get_field(5,9))
   wait_for_message("Port Space Lost!")

   -- Look for the other port space which is surrounded by closest allowed rocks
   -- and artifacts
   print("Checking detection of port space with closest possible blocking immovables")
   -- portdock can't be on border
   p1:conquer(map:get_field(12,7), 5)
   ship.scouting_direction = "ne"
   wait_for_message("Port Space")

   -- test port lost detection with an artifact
   print("Invalidating port space with an artifact")
   f = map:get_field(12,8)
   -- the tree may have already died
   if (f.immovable) then
      f.immovable:remove()
   end
   map:place_immovable("artifact00", f)
   wait_for_message("Port Space Lost!")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
