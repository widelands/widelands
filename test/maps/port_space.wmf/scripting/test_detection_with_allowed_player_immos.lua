run(function()
   --  This places allowed flags and roads around the port space owned by p1
   p1:conquer(map:get_field(7,10), 5)
   local flag = p1:place_flag(map:get_field(7,11))
   connected_road("normal", p1, flag, "tr,tr|tl,l|l,bl|bl,br|r,r")
   local ship = create_infrastructure(55, 10)
   game.desired_speed = 1000
   print("Checking detection of port space with existing roads")
   wait_for_message("Port Space")
   -- this creates a road that crosses the port space
   print("Invalidating port space")
   connected_road("normal", p1, flag, "tl,tl,l")
   wait_for_message("Port Space Lost!")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
