-- =======================================================================
--                            Secret Village Thread
-- =======================================================================


function village_thread()
   local plr = wl.Game().players[1]
   while not (plr:seen_field(wl.Game().map:get_field(52,39)) or
              plr:seen_field(wl.Game().map:get_field(58,10))) do
         sleep(6534)
   end

   reveal_village()

   local pts = scroll_smoothly_to(map:get_field(55, 25), 3000)
   campaign_message_box(msg_village)
   timed_scroll(array_reverse(pts), 10)

   sleep(1500)
end


--[[
   This is a village of poor but friendly people who have settled in a safe
   valley between two glaciers. They hunt and produce timber and grain but they
   do not have ores or even stones, so they are dependent on the infrequent
   merchant that may pass by and provide them with whatever they cannot
   produce on their own. Their only protection is a guard hut at each entrance
   to the valley. Therefore they realize that they may have to join a more
   powerful society for protection in order to stay alive in this world.

   A user that explores the map far from home will discover this village as a
   bonus. Doing so is not necessary for winning. The village can still be found
   when the scenario is already won.

   Technically the village is created instantly when the player sees any of the
   two entrances to the valley. But we place some trees and fields in various
   stages of growth to make it seem like the village has actually existed for
   some time. Some land ownership adjustments are made to ensure that the
   village owns all land between the glaciers.
--]]

function reveal_village()
   function force_map_immovables(list)
      local map = wl.Game().map
      for idx, id in ipairs(list) do
         local f = map:get_field(id[2], id[3])
         if f.immovable then
            pcall(f.immovable.remove, f.immovable)
         end
         map:place_immovable(id[1], f, id[4])
      end
   end

   force_map_immovables{
      { "spruce_summer_old", 55, 19 },
      { "larch_summer_pole", 58, 19 },
      { "birch_summer_mature", 58, 20 },
      { "larch_summer_old", 57, 21 },
      { "alder_summer_pole", 54, 22 },
      { "birch_summer_pole", 56, 24 },
      { "aspen_summer_old", 58, 24 },
      { "larch_summer_pole", 56, 25 },
      { "spruce_summer_old", 53, 27 },
      { "larch_summer_pole", 57, 27 },
      { "aspen_summer_mature", 52, 29 },
      { "birch_summer_pole", 54, 30 },
      { "beech_summer_old", 55, 30 },
      { "larch_summer_old", 56, 30 },
      { "field2", 56, 14, "barbarians" },
      { "field0s",57, 14, "barbarians" },
      { "field2", 54, 15, "barbarians" },
      { "field2", 57, 15, "barbarians" },
      { "field2", 54, 16, "barbarians" },
      { "field1", 57, 16, "barbarians" },
      { "field2", 58, 16, "barbarians" },
      { "field2", 54, 17, "barbarians" },
      { "field0", 55, 17, "barbarians" },
      { "field2", 57, 17, "barbarians" },
      { "field2", 55, 18, "barbarians" },
      { "field2", 57, 18, "barbarians" },
      { "field2", 53, 31, "barbarians" },
      { "field2", 54, 31, "barbarians" },
      { "field0", 55, 31, "barbarians" },
      { "field2", 56, 32, "barbarians" },
      { "field2", 52, 33, "barbarians" },
      { "field0s",55, 33, "barbarians" },
      { "field2", 56, 33, "barbarians" },
      { "field2", 53, 34, "barbarians" },
      { "field1", 54, 34, "barbarians" },
      { "field2", 56, 34, "barbarians" },
      { "field2", 53, 35, "barbarians" },
      { "field2", 55, 35, "barbarians" },
   }

   local plr = wl.Game().players[1]
   prefilled_buildings(plr,
      {"barbarians_sentry", 57, 9},
      {"barbarians_sentry", 52, 39},
      {"barbarians_hunters_hut", 56, 10},
      {"barbarians_gamekeepers_hut", 56, 12},
      {"barbarians_farm", 56, 16},
      {"barbarians_well", 54, 18},
      {"barbarians_bakery", 55, 20, wares = {wheat = 6, water = 6}},
      {"barbarians_lumberjacks_hut", 56, 21},
      {"barbarians_lumberjacks_hut", 55, 22},
      {"barbarians_lumberjacks_hut", 54, 24},
      {"barbarians_rangers_hut", 57, 24},
      {"barbarians_rangers_hut", 55, 25},
      {"barbarians_wood_hardener", 54, 26, wares = {log = 8}},
      -- to make it more realistic
      {"barbarians_warehouse", 53, 28,
         wares = {
            wheat = 20,
            log = 40,
            meat = 30
         }
      },
      {"barbarians_inn", 55, 28, wares = {barbarians_bread = 4, meat = 4}},
      {"barbarians_tavern", 57, 28, wares = {barbarians_bread = 4, meat = 4}},
      {"barbarians_well", 52, 30},
      {"barbarians_farm", 54, 33},
      {"barbarians_bakery", 51, 35, wares = {wheat = 6, water = 6}},
      {"barbarians_well", 52, 37}
   )

   -- Adjust the borders so that the village owns everything green
   local map = wl.Game().map
   plr:conquer(map:get_field(59, 16), 2)
   plr:conquer(map:get_field(57, 18), 2)
   plr:conquer(map:get_field(58, 19), 1)
   plr:conquer(map:get_field(58, 20), 1)
   plr:conquer(map:get_field(54, 15), 1)
   plr:conquer(map:get_field(54, 16), 1)
   plr:conquer(map:get_field(54, 20), 1)
   plr:conquer(map:get_field(54, 22), 1)
   plr:conquer(map:get_field(57, 23), 1)
   plr:conquer(map:get_field(58, 24), 1)
   plr:conquer(map:get_field(57, 27), 1)
   plr:conquer(map:get_field(56, 31), 1)
   plr:conquer(map:get_field(56, 33), 1)
   plr:conquer(map:get_field(52, 32), 1)

   -- Build roads
   -- Start at northern sentry
   connected_road(plr, map:get_field(58, 10).immovable,
      "w,sw|se,sw|e,se|se,se|sw,sw|sw,w|sw,sw|se,sw|sw,sw|se,sw|" ..
      "sw,sw|sw,sw|sw,sw|se,se,sw|e,e|sw,sw|se,sw|")

   connected_road(plr, map:get_field(57, 25).immovable, "sw,w|sw,w")
   connected_road(plr, map:get_field(57, 29).immovable, "w,w|w,w")
   connected_road(plr, map:get_field(55, 34).immovable, "sw,sw")
   connected_road(plr, map:get_field(57, 22).immovable, "sw,w")
   connected_road(plr, map:get_field(54, 19).immovable, "sw,se,e")
   connected_road(plr, map:get_field(56, 17).immovable, "sw,se")
end

run(village_thread)
