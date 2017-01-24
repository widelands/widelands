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

   local prior_center = scroll_to_field(map:get_field(55, 25))
   campaign_message_box(msg_village)
   scroll_to_map_pixel(prior_center)

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
      { "spruce_summer_old", 55, 19, "world" },
      { "larch_summer_pole", 58, 19, "world" },
      { "birch_summer_mature", 58, 20, "world" },
      { "larch_summer_old", 57, 21, "world" },
      { "alder_summer_pole", 54, 22, "world" },
      { "birch_summer_pole", 56, 24, "world" },
      { "aspen_summer_old", 58, 24, "world" },
      { "larch_summer_pole", 56, 25, "world" },
      { "spruce_summer_old", 53, 27, "world" },
      { "larch_summer_pole", 57, 27, "world" },
      { "aspen_summer_mature", 52, 29, "world" },
      { "birch_summer_pole", 54, 30, "world" },
      { "beech_summer_old", 55, 30, "world" },
      { "larch_summer_old", 56, 30, "world" },
      { "field_harvested", 56, 14, "tribes" },
      { "field_small",57, 14, "tribes" },
      { "field_harvested", 54, 15, "tribes" },
      { "field_harvested", 57, 15, "tribes" },
      { "field_harvested", 54, 16, "tribes" },
      { "field_ripe", 57, 16, "tribes" },
      { "field_harvested", 58, 16, "tribes" },
      { "field_harvested", 54, 17, "tribes" },
      { "field_medium", 55, 17, "tribes" },
      { "field_harvested", 57, 17, "tribes" },
      { "field_harvested", 55, 18, "tribes" },
      { "field_harvested", 57, 18, "tribes" },
      { "field_harvested", 53, 31, "tribes" },
      { "field_harvested", 54, 31, "tribes" },
      { "field_medium", 55, 31, "tribes" },
      { "field_harvested", 56, 32, "tribes" },
      { "field_harvested", 52, 33, "tribes" },
      { "field_small",55, 33, "tribes" },
      { "field_harvested", 56, 33, "tribes" },
      { "field_harvested", 53, 34, "tribes" },
      { "field_ripe", 54, 34, "tribes" },
      { "field_harvested", 56, 34, "tribes" },
      { "field_harvested", 53, 35, "tribes" },
      { "field_harvested", 55, 35, "tribes" },
   }

   local plr = wl.Game().players[1]
   prefilled_buildings(plr,
      {"barbarians_sentry", 57, 9},
      {"barbarians_sentry", 52, 39},
      {"barbarians_hunters_hut", 56, 10},
      {"barbarians_gamekeepers_hut", 56, 12},
      {"barbarians_farm", 56, 16},
      {"barbarians_well", 54, 18},
      {"barbarians_bakery", 55, 20, inputs = {wheat = 6, water = 6}},
      {"barbarians_lumberjacks_hut", 56, 21},
      {"barbarians_lumberjacks_hut", 55, 22},
      {"barbarians_lumberjacks_hut", 54, 24},
      {"barbarians_rangers_hut", 57, 24},
      {"barbarians_rangers_hut", 55, 25},
      {"barbarians_wood_hardener", 54, 26, inputs = {log = 8}},
      -- to make it more realistic
      {"barbarians_warehouse", 53, 28,
         wares = {
            wheat = 20,
            log = 40,
            meat = 30
         }
      },
      {"barbarians_inn", 55, 28, inputs = {barbarians_bread = 4, meat = 4}},
      {"barbarians_tavern", 57, 28, inputs = {barbarians_bread = 4, meat = 4}},
      {"barbarians_well", 52, 30},
      {"barbarians_farm", 54, 33},
      {"barbarians_bakery", 51, 35, inputs = {wheat = 6, water = 6}},
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
