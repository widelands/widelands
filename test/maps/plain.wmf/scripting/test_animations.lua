run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hq = p1:get_buildings("barbarians_headquarters")[1]

   hq:set_workers("barbarians_file_animation", 5)
   assert_equal(5, hq:get_workers("barbarians_file_animation"))

   -- This mine needs a miner, a chief miner and a master miner
   p1:place_road(p1:place_flag(map:get_field(18,22)), "br", "br", "br", "br", "r", "r", "r")
   p1:place_flag(map:get_field(20,26))
   local building = p1:place_building("barbarians_spritesheet_animation", wl.Game().map:get_field(17, 21), false, true)

   -- NOCOM make the worker walk out of the building and idle

   sleep(1000 * 10)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
