include "test/scripting/stable_save.lua"

run(function()
   sleep(2000)
   place_markets()

   assert_equal(0, #game.trades)
   local id = market_p2:propose_trade(p1, 5, { log = 3 }, { granite = 2, iron = 1 })
   assert_equal(1, #game.trades)
   assert_equal("proposed", game.trades[1].state)
   assert_nil(game.trades[1].receiver)
   assert_not_nil(game.trades[1].initiator)

   sleep(2000)
   market_p1:accept_trade(id)

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

   sleep(2 * 60 * 1000)
   stable_save(game, "trading", 50 * 1000)

   sleep(1 * 60 * 1000)
   assert_equal(1, #game.trades)
   assert_equal("running", game.trades[1].state)
   assert_equal(5, game.trades[1].num_batches)
   assert_equal(3, game.trades[1].items_to_send.log)
   assert_equal(1, game.trades[1].items_to_receive.iron)
   assert_equal(2, game.trades[1].items_to_receive.granite)
   assert_equal(1, game.trades[1].receiving_player)
   assert_not_nil(game.trades[1].receiver)

   -- We await until one ware we trade has the right count for one player.
   -- Then, we'll sleep half as long as we already waited to make sure that no
   -- additional batches are shipped. Then we check all stocks for the correct
   -- numbers.
   local start_time = game.time
   while p2:get_wares("iron") - p2_initial["iron"] < 5 do
      sleep(10000)
   end

   sleep(math.ceil((game.time - start_time) / 2))

   assert_equal(0, #game.trades)

   assert_equal(5, p2:get_wares("iron") - p2_initial["iron"])
   assert_equal(10, p2:get_wares("granite") - p2_initial["granite"])
   assert_equal(-15, p2:get_wares("log") - p2_initial["log"])

   assert_equal(-5, p1:get_wares("iron") - p1_initial["iron"])
   assert_equal(-10, p1:get_wares("granite") - p1_initial["granite"])
   assert_equal(15, p1:get_wares("log") - p1_initial["log"])

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
