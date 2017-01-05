run(function()
   game = wl.Game()
   map = game.map
   p1 = game.players[1]
   log_starting_count = p1.produced_wares_count["log"]
   assert_equal(0, log_starting_count)

   create_one_ship()

   test_transporting_works()
   log_final_count = p1.produced_wares_count["log"]
   assert(log_final_count > log_starting_count + 5)
   
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
