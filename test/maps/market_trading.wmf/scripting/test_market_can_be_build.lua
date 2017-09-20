run(function()
   sleep(2000)

   market = p2:place_building("barbarians_market", map:get_field(35, 25), true, true)
   connected_road(p2, market.flag, "l,l,l|", true)

   while #p2:get_buildings("barbarians_market") == 0 do
      sleep(10000)
   end
   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
