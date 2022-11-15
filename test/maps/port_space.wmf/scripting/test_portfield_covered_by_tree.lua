run(function()
   --  This places to tower so that the port just fits the space.
   map:place_immovable("alder_summer_old", map:get_field(7,10))
   local ship = create_infrastructure(59, 10)
   game.desired_speed = 1000
   wait_for_message("Port Space")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
