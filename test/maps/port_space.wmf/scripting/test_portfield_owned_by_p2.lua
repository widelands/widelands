run(function()
   -- This places to tower so that the full island including the port space is
   -- clearly owned by player1, no chance to build a port for p2.
   local ship = create_infrastructure(63, 10)
   wait_for_message("Land Ahoy!")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
