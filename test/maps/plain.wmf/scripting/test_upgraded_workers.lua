run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hq = p1:get_buildings("barbarians_headquarters")[1]

   hq:set_workers("barbarians_miner_chief", 5)
   assert_equal(5, hq:get_workers("barbarians_miner_chief"))

   -- This mine needs a miner, a chief miner and a master miner
   p1:place_road(p1:place_flag(map:get_field(25,22)), "bl", "bl", "bl", "bl")
   local mine = p1:place_building("barbarians_coalmine_deeper", wl.Game().map:get_field(24, 21), false, true)

   sleep(1000)

   -- 2 of the chief miners should have been requested by the mine now,
   -- one of them acting as a basic miner
   assert_equal(3, hq:get_workers("barbarians_miner_chief"))

   hq:set_workers("barbarians_miner_master", 1)
   sleep(1000)

   -- The master miner has filled the remaining slot
   assert_equal(0, hq:get_workers("barbarians_miner_master"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
