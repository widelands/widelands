run(function()
print("NOCOMNOCOM AAA")
   create_two_ships()
print("NOCOMNOCOM BBB")


   sleep(100)
   game.desired_speed = 10 * 1000

print("NOCOMNOCOM CCC")
   -- Send expedition to port space.
   port:start_expedition()
   wait_for_message("Expedition")
   assert_equal(1, p1:get_workers("barbarians_builder"))
   sleep(500)
print("NOCOMNOCOM DDD")


   if first_ship.state=="exp_waiting" then
      expedition_ship=first_ship
   elseif second_ship.state=="exp_waiting" then
      expedition_ship=second_ship
   else
      assert(false)
   end
print("NOCOMNOCOM EEE")

   expedition_ship.island_explore_direction="ccw"
   sleep(2000)
   assert_equal("ccw",expedition_ship.island_explore_direction)
print("NOCOMNOCOM FFF")
   wait_for_message("Port Space")
print("NOCOMNOCOM GGG")
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))

print("NOCOMNOCOM HHH *** SAVING")
   stable_save(game, "reached_port_space", 10 * 1000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
print("NOCOMNOCOM III *** RELOADED?")

   cancel_expedition_in_shipwindow(expedition_ship)
print("NOCOMNOCOM JJJ")
   sleep(50000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
print("NOCOMNOCOM KKK")
   check_wares_in_port_are_all_there()
print("NOCOMNOCOM LLL")

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()



end)
