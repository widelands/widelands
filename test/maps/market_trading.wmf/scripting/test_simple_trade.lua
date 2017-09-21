run(function()
   sleep(2000)
   place_markets()

   market_p1:propose_trade(market_p2, 5, { log = 3 }, { granite = 2 })
end)

