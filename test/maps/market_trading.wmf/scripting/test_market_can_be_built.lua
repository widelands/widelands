run(function()
   sleep(2000)

   market = p2:place_building("barbarians_market", map:get_field(31, 27), true, true)
   connected_road("normal", p2, market.flag, "tr,tl|", true)

   while #p2:get_buildings("barbarians_market") == 0 do
      sleep(10000)
   end
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
