run(function()
   create_two_ships()


   sleep(100)
   game.desired_speed = 10 * 1000

   -- Send expedition to port space.
   port:start_expedition()
   wait_for_message("Expedition")
   assert_equal(1, p1:get_workers("barbarians_builder"))
   sleep(500)


   if first_ship.state=="exp_waiting" then
      expedition_ship=first_ship
   elseif second_ship.state=="exp_waiting" then
      expedition_ship=second_ship
   else
      assert(false)
   end

   expedition_ship.island_explore_direction="ccw"
   sleep(2000)
   assert_equal("ccw",expedition_ship.island_explore_direction)
   wait_for_message("Port Space")
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   stable_save("reached_port_space")
   assert_equal(1, p1:get_workers("barbarians_builder"))

   cancel_expedition_in_shipwindow(expedition_ship)
   sleep(20000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
   check_wares_in_port_are_all_there()

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()



end)
