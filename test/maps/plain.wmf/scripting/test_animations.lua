run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hq = p1:get_buildings("barbarians_headquarters")[1]

   hq:set_workers("barbarians_file_animation", 5)
   assert_equal(5, hq:get_workers("barbarians_file_animation"))

   -- Place a building that has a spritesheet animation
   p1:place_road("normal", p1:place_flag(map:get_field(18,22)), "br", "br", "br", "br", "r", "r", "r")
   p1:place_flag(map:get_field(20,26))
   local building = p1:place_building("barbarians_spritesheet_animation", wl.Game().map:get_field(17, 21), false, true)

   -- Make the worker (file animations) walk out of the building and idle for a few times
   sleep(5 * 40000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
