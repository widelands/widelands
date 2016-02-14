run(function()
   --  This places to tower so that the port just fits the space.
   local ship = create_infrastructure(60, 10)
   game.desired_speed = 1000
   wait_for_message("Port Space")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
