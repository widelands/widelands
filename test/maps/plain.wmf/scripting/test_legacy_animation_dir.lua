-- This is a copy of test_animations.lua, with the difference being, that these animations
-- use the deprecated 'directory' attribute for each animation instead of 'animation_directory',
-- in order to ensure this attribute continues to work.
-- The copied animations use the 'directory' attribute to reference the original animations
-- of the two other directories.
run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hq = p1:get_buildings("barbarians_headquarters")[1]

   hq:set_workers("barbarians_legacy_file_animation_dir", 5)
   assert_equal(5, hq:get_workers("barbarians_legacy_file_animation_dir"))

   -- Place a building that has a spritesheet animation
   p1:place_road("normal", p1:place_flag(map:get_field(18,22)), "br", "br", "br", "br", "r", "r", "r")
   p1:place_flag(map:get_field(20,26))
   local building = p1:place_building("barbarians_legacy_spritesheet_animation_dir", wl.Game().map:get_field(17, 21), false, true)

   -- Make the worker (file animations) walk out of the building and idle for a few times
   sleep(5 * 40000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
