-- Main logic.
run(function()
   sleep(100)
   game.desired_speed = 1000

   -- Start and immediately cancel an expedition.
   print("---- 1 -----")
   port:start_expedition()
   sleep(1000)
   assert(port.expedition_in_progress)
   port:cancel_expedition()
   sleep(1000)
   assert(not port.expedition_in_progress)
   sleep(1000)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
