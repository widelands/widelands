run(function()
   game = wl.Game()
   map = game.map
   p1 = game.players[1]

   create_one_ship()

   test_transporting_works()
end)
