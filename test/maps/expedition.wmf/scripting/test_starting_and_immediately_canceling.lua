-- Main logic.
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start and immediately cancel an expedition.
   print("---- 1 -----")
   start_expedition()
   cancel_expedition_in_port()
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
