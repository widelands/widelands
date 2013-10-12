run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Send expedition to port space.
   start_expedition()
   wait_for_message("Expedition ready")
   assert_equal(1, p1:get_workers("builder"))
   sleep(500)

   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   wait_for_message("Port space found")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   stable_save("reached_port_space")
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
