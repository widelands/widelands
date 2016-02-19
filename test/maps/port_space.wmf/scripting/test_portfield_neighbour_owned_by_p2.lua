run(function()
   -- This places to tower so that bug 1542703 triggered. Now, we do not allow
   -- a portspace.
   local ship = create_infrastructure(61, 10)
   wait_for_message("Land Ahoy!")
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
