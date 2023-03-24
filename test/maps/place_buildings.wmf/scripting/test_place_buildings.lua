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

function place(building, x, y, force)
   if not force then
      p1:conquer(map:get_field(x + 2, y), 8)
   end
   local b = p1:place_building(building, map:get_field(x, y), false, force)
   assert_equal(building, b.descr.name)
   local cs = p1:place_building(building, map:get_field(x + 5, y), true, force)
   assert_equal("constructionsite", cs.descr.name)
end

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
         place(bdg_to_place[s], 10 * x, y, force)
         force = true
      end
   end

   place(bdg_to_place.over, 40, y_trees_edge, false)

   sleep(1000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
