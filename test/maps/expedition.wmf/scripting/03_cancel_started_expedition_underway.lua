run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start a new expedition.
   start_expedition()
   wait_for_message("Expedition ready")
   game.desired_speed = 10 * 1000
   sleep(10000)

   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   sleep(8000)

   stable_save("sailing")
   assert_equal(1, p1:get_workers("builder"))

   cancel_expedition_in_shipwindow(first_ship)
   sleep(20000)
   assert_equal(1, p1:get_workers("builder"))
   check_wares_in_port_are_all_there()

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
