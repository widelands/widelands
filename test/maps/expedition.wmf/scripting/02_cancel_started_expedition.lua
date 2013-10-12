run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start a new expedition.
   start_expedition()
   wait_for_message("Expedition ready")
   game.desired_speed = 10 * 1000
   sleep(10000)

   stable_save("ready_to_sail")

   sleep(10000)
   assert_equal(1, p1:get_workers("builder"))

   -- Now cancel the expedition before it even got send out.
   cancel_expedition_in_shipwindow()
   sleep(100)
   assert_equal(1, p1:get_workers("builder"))
   sleep(8000)  -- ship needs a while to get wares back.
   check_wares_in_port_are_all_there()
   assert_equal(1, p1:get_workers("builder"))

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
