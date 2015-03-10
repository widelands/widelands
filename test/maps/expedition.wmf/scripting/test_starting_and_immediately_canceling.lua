-- Main logic.
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start and immediately cancel an expedition.
   print("---- 1 -----")
   port:start_expedition()
   port:cancel_expedition()
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
