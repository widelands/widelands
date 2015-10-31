-- Main logic.
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start and immediately cancel an expedition.
   print("---- 1 -----")
   port:start_expedition()
   sleep(500)
   assert(port.expedition_in_progress)
   port:cancel_expedition()
   sleep(500)
   assert(not port.expedition_in_progress)
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)