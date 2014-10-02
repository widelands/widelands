-- =======================================================================
--                            Main mission thread
-- =======================================================================

include "map:scripting/mission_thread_texts.lua"
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

quarry_done = false
enhance_buildings_done = false
build_materials_done = false
cattle_farm_done = false

-- ==============================================
-- First messages player at beginning of mission
-- ==============================================
function introduction_thread()
   sleep(2000)

   campaign_message_box(briefing_msg_1)
   campaign_message_box(briefing_msg_2)
   campaign_message_box(briefing_msg_3)

   campaign_message_box(order_msg_1)
   campaign_message_box(order_msg_2)

   -- Reveal the rocks
   local obj = add_campaign_objective(obj_claim_northeastern_rocks)

   local rocks = wl.Game().map:get_field(27, 48)
   local intermediate_point = wl.Game().map:get_field(31,12)
   plr:reveal_fields(rocks:region(6))
   local way1 = scroll_smoothly_to(intermediate_point, 1500)
   local way2 = scroll_smoothly_to(rocks, 1500)

   campaign_message_box(order_msg_3)
   campaign_message_box(order_msg_4)

   -- Move back to HQ
   timed_scroll(array_reverse(way2), 10)
   timed_scroll(array_reverse(way1), 10)

   -- Now, wait till the quarry comes up
   local f = wl.Game().map:get_field(27,48):region(6)
   while not check_for_buildings(plr, { quarry = 1 }, f) do
      sleep(5000)
   end
   obj.done = true

   campaign_message_box(order_msg_5_quarry)

   quarry_done = true
end

-- ==================================
-- How to mine & produce food thread
-- ==================================
function mines_and_food_thread()
   local f1 = wl.Game().map:get_field(30, 15)
   local f2 = wl.Game().map:get_field(24, 61)

   -- Sleep until we see the mountains
   while not plr:seen_field(f1) and not plr:seen_field(f2) do
      sleep(4000)
   end

   -- Send a msg and add the objective
   campaign_message_box(order_msg_6_geologist)
   o = add_campaign_objective(obj_build_mines)
   plr:allow_buildings{
      "coalmine",
      "oremine",
      "goldmine",
      "granitemine"
   }

   -- Wait for completion
   while not check_for_buildings(plr, {coalmine = 1, oremine = 1}) do
      sleep(5000)
   end
   o.done = true
   campaign_message_box(order_msg_7_mines_up)
   campaign_message_box(order_msg_8_mines_up)
   campaign_message_box(order_msg_9_hunter)
   campaign_message_box(order_msg_10_bread)

   local obj_bf = add_campaign_objective(obj_basic_food)
   -- The function to check for completeness
   run(function()
      local tavern_msg_done = nil
      local hunter_msg_done = nil
      while true do
         local rv = plr:get_buildings{
            "hunters_hut", "gamekeepers_hut", "tavern"
         }
         if #rv.hunters_hut >= 1 and not hunter_msg_done then
            campaign_message_box(order_msg_11_basic_food_began)
            hunter_msg_done = true
         end
         if #rv.tavern >= 1 and not tavern_msg_done then
            campaign_message_box(order_msg_13_tavern)
            tavern_msg_done = true
         end
         if #rv.hunters_hut >= 1 and #rv.gamekeepers_hut >= 1
                  and #rv.tavern >= 1 then break end
         sleep(5331)
      end
      obj_bf.done = true
   end)

   local obj_farming = add_campaign_objective(obj_begin_farming)

   -- Start the cattlefarm thread
   run(cattle_farm)

   run(function()
      while 1 do
         local rv = plr:get_buildings{"well", "bakery", "farm"}
         if #rv.well >= 1 and #rv.bakery >= 1 and #rv.farm >= 1 then
            break
         end
         sleep(4234)
      end
      campaign_message_box(order_msg_12_farming_began)
      obj_farming.done = true
   end)

   -- Enable food production
   plr:allow_buildings{
      "hunters_hut",
      "fishers_hut",
      "gamekeepers_hut",
      "tavern",
      "farm",
      "well",
      "bakery",
   }

   while not (obj_bf.done and obj_farming.done) do
      sleep(6231)
   end

   -- Ready to build refiner stuff
   campaign_message_box(order_msg_14_refine_ore)
   plr:allow_buildings{"smelting_works"}
   o = add_campaign_objective(obj_refine_ores)
   while #plr:get_buildings("smelting_works") < 1 do
      sleep(6223)
   end
   o.done = true

   -- Information about making mines deeper
   campaign_message_box(order_msg_15_mines_exhausted)
   plr:allow_buildings{ "deep_coalmine", "inn", "micro-brewery" }
   -- objective.check will make sure that this i finished
   local obj = add_campaign_objective(obj_enhance_buildings)

   run(function()
      while not check_for_buildings(plr,
         { inn = 1, deep_coalmine = 1, ["micro-brewery"] = 1 })
      do
         sleep(5742)
      end
      obj.done = true
      enhance_buildings_done = true
   end)

end

-- =================================
-- Better building materials thread
-- =================================
function build_materials_thread()
   local plr = wl.Game().players[1]

   -- Wait for a barrier or sentry to be build
   while true do
      local rv = plr:get_buildings{"sentry", "barrier"}
      if #rv.sentry + #rv.barrier > 0 then
         break
      end
      sleep(5421)
   end

   campaign_message_box(order_msg_16_blackwood)
   plr:allow_buildings{"hardener"}
   local o = add_campaign_objective(obj_better_material_1)
   while #plr:get_buildings("hardener") < 1 do sleep(5421) end
   o.done = true

   campaign_message_box(order_msg_17_grindstone)
   plr:allow_buildings{"lime_kiln", "well", "charcoal_kiln"}
   o = add_campaign_objective(obj_better_material_2)
   -- Wait for the buildings to be build
   while true do
      local rv = plr:get_buildings{"lime_kiln", "well",
         "coalmine", "deep_coalmine", "charcoal_kiln"}
      if (#rv.lime_kiln > 0 and #rv.well > 0) and
         (#rv.coalmine + #rv.deep_coalmine + #rv.charcoal_kiln > 0) then
         break
      end
      sleep(5421)
   end
   o.done = true

   campaign_message_box(order_msg_18_reed_yard)
   plr:allow_buildings{"reed_yard"}
   o = add_campaign_objective(obj_better_material_3)
   while #plr:get_buildings("reed_yard") < 1 do sleep(5421) end

   campaign_message_box(order_msg_19_all_material)
   o.done = true

   build_materials_done = true
end

-- ==================
-- Cattlefarm thread
-- ==================
function cattle_farm()
   while not check_for_buildings(plr, { farm = 1, well = 1 }) do
      sleep(7834)
   end

   campaign_message_box(msg_cattlefarm_00)

   local o = add_campaign_objective(obj_build_cattlefarm)
   plr:allow_buildings{"cattlefarm"}

   while not check_for_buildings(plr, { cattlefarm = 1 }) do
      sleep(2323)
   end
   o.done = true

   cattle_farm_done = true
end


-- ======================
-- Throns story messages
-- ======================
function story_messages_thread()
   wake_me(180 * 1000)
   campaign_message_box(msg_story_2)

   wake_me(600 * 1000)
   campaign_message_box(msg_story_1)
end

-- =================
-- Mission complete
-- =================
function mission_complete_thread()
   while not (build_materials_done and quarry_done
            and enhance_buildings_done and cattle_farm_done) do
         sleep(10000)
   end

   campaign_message_box(msg_mission_complete)
   plr:reveal_scenario("barbariantut02")
end

-- ===============
-- Village thread
-- ===============
function village_thread()
   local plr = wl.Game().players[1]
   while not (plr:seen_field(wl.Game().map:get_field(52,39)) or
              plr:seen_field(wl.Game().map:get_field(58,10))) do
         sleep(6534)
   end

   reveal_village()

   pts = scroll_smoothly_to(wl.Game().map:get_field(55, 25), 3000)

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
   bonus. Doing so is not necessary for winning.

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
      {"sentry", 57, 9},
      {"sentry", 52, 39},
      {"hunters_hut", 56, 10},
      {"gamekeepers_hut", 56, 12},
      {"farm", 56, 16},
      {"well", 54, 18},
      {"bakery", 55, 20, wares = {wheat=6, water=6}},
      {"lumberjacks_hut", 56, 21},
      {"lumberjacks_hut", 55, 22},
      {"lumberjacks_hut", 54, 24},
      {"rangers_hut", 57, 24},
      {"rangers_hut", 55, 25},
      {"hardener", 54, 26, wares = {log = 8}},
      {"warehouse", 53, 28},
      {"inn", 55, 28, wares = {pittabread = 4, meat = 4}},
      {"tavern", 57, 28, wares = {pittabread=4, meat = 4}},
      {"well", 52, 30},
      {"farm", 54, 33},
      {"bakery", 51, 35, wares = {wheat = 6, water = 6}},
      {"well", 52, 37}
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

run(introduction_thread)
run(mines_and_food_thread)
run(build_materials_thread)
run(story_messages_thread)
run(village_thread)

run(mission_complete_thread)
