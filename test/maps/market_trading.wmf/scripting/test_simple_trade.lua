run(function()
   sleep(2000)
   place_markets()
   market_p2:propose_trade(market_p1, 5, { log = 3 }, { granite = 2, iron = 1 })

   local p1_initial = {
      iron = p1:get_wares("iron"),
      log = p1:get_wares("log"),
      granite = p1:get_wares("granite"),
   }
   local p2_initial = {
      iron = p1:get_wares("iron"),
      log = p1:get_wares("log"),
      granite = p1:get_wares("granite"),
   }

   -- We await until one ware we trade has the right count for one player.
   -- Then, we'll sleep half as long as we already waited to make sure that no
   -- additional batches are shipped. Then we check all stocks for the correct
   -- numbers.
   local start_time = game.time
   while p2:get_wares("iron") - p2_initial["iron"] < 5 do
      sleep(10000)
   end

   sleep(math.ceil((game.time - start_time) / 2))

   assert_equal(5, p2:get_wares("iron") - p2_initial["iron"])
   assert_equal(10, p2:get_wares("granite") - p2_initial["granite"])
   assert_equal(-15, p2:get_wares("log") - p2_initial["log"])

   assert_equal(-5, p1:get_wares("iron") - p1_initial["iron"])
   assert_equal(-10, p1:get_wares("granite") - p1_initial["granite"])
   assert_equal(15, p1:get_wares("log") - p1_initial["log"])

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
