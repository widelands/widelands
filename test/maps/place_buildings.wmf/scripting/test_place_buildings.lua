local sizes = { "big", "medium", "small", "over" }

local y_coords = {
   0,   -- free
   20,  -- stones
   32,  -- trees
   50   -- water
}

local y_trees_edge = 42

local bdg_to_place = {
   big = "amazons_cassava_plantation",
   medium = "amazons_warriors_gathering",
   small = "amazons_patrol_post",
   over = "amazons_treetop_sentry"
}

run(function()
   sleep(1000)

   local force
   for x,s in ipairs(sizes) do
      if s == "over" then
        force = true
      else
        force = false
      end
      for j,y in ipairs(y_coords) do
         if not force then
            p1:conquer(map:get_field(10 * x + 2, y), 8)
         end
         p1:place_building(bdg_to_place[s], map:get_field(10 * x, y), false, force)
         p1:place_building(bdg_to_place[s], map:get_field(10 * x + 5, y), true, force)
         force = true
      end
   end

   p1:conquer(map:get_field(42, y_trees_edge), 8)
   p1:place_building(bdg_to_place.over, map:get_field(40, y_trees_edge), false, false)
   p1:place_building(bdg_to_place.over, map:get_field(45, y_trees_edge), true, false)

   sleep(1000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
