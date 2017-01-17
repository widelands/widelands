run(function()
   game = wl.Game()
   egbase = game
   player1 = egbase.players[1]
   sleep(1000)

    prefilled_buildings(p1,
    { "barbarians_rangers_hut", 19, 23, workers = { barbarians_ranger = 1 } },
    { "barbarians_rangers_hut", 23, 29, workers = { barbarians_ranger = 1 } },
    { "barbarians_lumberjacks_hut", 20, 29, workers = { barbarians_lumberjack = 1 } }
   )

   starting_count = player1:get_produced_wares_count('log')
   -- No logs produced on the begining
   assert_equal(starting_count, 0)

   game.desired_speed = 50000

   while (starting_count == player1:get_produced_wares_count('log')) do
     sleep(1000)
   end
   
   -- some logs produced by now
   assert(player1:get_produced_wares_count('log') > 0)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
