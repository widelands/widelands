run(function()
   sleep(2000)

   market = p2:place_building("barbarians_market", map:get_field(31, 28), true, true)
   connected_road("normal", p2, market.flag, "tr,tr,tl|", true)

   while #p2:get_buildings("barbarians_market") == 0 do
      sleep(10000)
   end

   market = p2:get_buildings("barbarians_market")[1]
   assert_equal("barbarians_carrier", market.descr.local_carrier)
   assert_equal("barbarians_ox", market.descr.trade_carrier)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
