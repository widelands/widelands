run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start an expedition, but let them carry some wares into it. This also
   -- gives the builder enough time to walk over.
   start_expedition()
   sleep(50000)
   stable_save("cancel_in_port")
   assert_equal(1, p1:get_workers("builder"))

   cancel_expedition_in_port()
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
