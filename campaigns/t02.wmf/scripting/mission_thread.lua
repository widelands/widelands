-- =======================================================================
--                            Main mission thread
-- =======================================================================

use("map", "mission_thread_texts")
use("aux", "ui")
use("aux", "table")

quarry_done = false
enhance_buildings_done = false
build_materials_done = false

function send_msg(t)
   p:message_box(t.title, t.body, t)
end

function add_obj(t)
   local o = p:add_objective(t.name, t.title, t.body)
   if t.check then
      run(t.check, o)
   end
   return o
end


-- ==============================================
-- First messages player at beginning of mission
-- ==============================================
function introduction_thread()
   sleep(2000)

   send_msg(briefing_msg_1)
   send_msg(briefing_msg_2)
   send_msg(briefing_msg_3)

   send_msg(order_msg_1)
   send_msg(order_msg_2)

   -- Reveal the rocks
   add_obj(obj_claim_northeastern_rocks)

   local rocks = wl.map.Field(27, 48)
   p:reveal_fields(rocks:region(6))
   pts = scroll_smoothly_to(rocks, 3000)

   send_msg(order_msg_3)
   send_msg(order_msg_4)

   -- Move back to HQ
   timed_scroll(array_reverse(pts), 10)
end

-- ==================================
-- How to mine & produce food thread
-- ==================================
function mines_and_food_thread()
   local f1 = wl.map.Field(30, 15)
   local f2 = wl.map.Field(24, 61)

   -- Sleep until we see the mountains
   while not p:seen_field(f1) and not p:seen_field(f2) do
      sleep(4000)
   end

   -- Send a msg and add the objective
   send_msg(order_msg_6_geologist)
   o = add_obj(obj_build_mines)
   p:allow_buildings{
      "coalmine",
      "oremine",
      "goldmine",
      "granitemine"
   }

   -- Wait for completion
   while not check_for_buildings(p, {coalmine = 1, oremine = 1}) do
      sleep(5000)
   end
   o.done = true
   send_msg(order_msg_7_mines_up)
   send_msg(order_msg_8_mines_up)
   send_msg(order_msg_9_hunter)
   send_msg(order_msg_10_bread)

   local obj_bf = add_obj(obj_basic_food)
   local obj_farming = add_obj(obj_begin_farming)

   -- Enable food production
   p:allow_buildings{
      "hunters_hut",
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
   send_msg(order_msg_14_refine_ore)
   p:allow_buildings{"smelting_works"}
   o = add_obj(obj_refine_ores)
   while #p:get_buildings("smelting_works") < 1 do
      sleep(6223)
   end
   o.done = true

   -- Information about making mines deeper
   send_msg(order_msg_15_mines_exhausted)
   p:allow_buildings{ "deep_coalmine", "inn" }
   -- objective.check will make sure that this i finished
   add_obj(obj_enhance_buildings)

end

-- =================================
-- Better building materials thread
-- =================================
function build_materials_thread()
   local p = wl.game.Player(1)

   -- Wait for a barrier or a sentry to be build
   while true do
      local rv = p:get_buildings{"sentry", "stronghold"}
      if #rv.sentry + #rv.stronghold > 0 then
         break
      end
      sleep(5421)
   end

   send_msg(order_msg_16_blackwood)
   p:allow_buildings{"hardener"}
   local o = add_obj(obj_better_material_1)
   while #p:get_buildings("hardener") < 1 do sleep(5421) end
   o.done = true

   send_msg(order_msg_17_grindstone)
   p:allow_buildings{"lime_kiln", "well", "burners_house"}
   o = add_obj(obj_better_material_2)
   -- Wait for the buildings to be build
   while true do
      local rv = p:get_buildings{"lime_kiln", "well",
         "coalmine", "deep_coalmine", "burners_house"}
      if (#rv.lime_kiln > 0 and #rv.well > 0) and
         (#rv.coalmine + #rv.deep_coalmine + #rv.burners_house > 0) then
         break
      end
      sleep(5421)
   end
   o.done = true

   send_msg(order_msg_18_fernery)
   p:allow_buildings{"fernery"}
   o = add_obj(obj_better_material_3)
   while #p:get_buildings("fernery") < 1 do sleep(5421) end

   send_msg(order_msg_19_all_material)
   o.done = true

   build_materials_done = true
end

-- ======================
-- Throns story messages
-- ======================
function story_messages_thread()
   wake_me(180 * 1000)
   send_msg(msg_story_2)

   wake_me(600 * 1000)
   send_msg(msg_story_1)
end

function mission_complete_thread()
   while not (build_materials_done and quarry_done
            and enhance_buildings_done ) do
         sleep(10000)
   end

   send_msg(msg_mission_complete)
   p:reveal_scenario("barbariantut02")
end

-- ===============
-- Village thread
-- ===============
function village_thread()
   local p = wl.game.Player(1)
   while not (p:seen_field(wl.map.Field(52,39)) or
              p:seen_field(wl.map.Field(58,10))) do
         sleep(6534)
   end

   reveal_village()

   pts = scroll_smoothly_to(wl.map.Field(55, 25), 3000)

   send_msg(msg_village)

   timed_scroll(array_reverse(pts), 10)
   sleep(1500)
end


--[[
   This is a village of poor but friendly people who have settled in a safe
   valley between two glaciers. They hunt and produce timber and grain but they
   do not have ores or even stones, so they are dependent on the infrequent
   merchant that may pass by and provide them with whatever they can not
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
      for idx, id in ipairs(list) do
         local f = wl.map.Field(id[2], id[3])
         if f.immovable then
            pcall(f.immovable.remove, f.immovable)
         end
         wl.map.create_immovable(id[1], f, id[4])
      end
   end

   force_map_immovables{
      { "tree3", 55, 19 },
      { "tree7_s", 58, 19 },
      { "tree5_m", 58, 20 },
      { "tree7", 57, 21 },
      { "tree4_s", 54, 22 },
      { "tree5_s", 56, 24 },
      { "tree1", 58, 24 },
      { "tree7_s", 56, 25 },
      { "tree3", 53, 27 },
      { "tree7_s", 57, 27 },
      { "tree1_m", 52, 29 },
      { "tree5", 54, 30 },
      { "tree6", 55, 30 },
      { "tree7", 56, 30 },
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

   local p = wl.game.Player(1)
   prefilled_buildings(p,
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
      {"hardener", 54, 26, wares = {trunk = 8}},
      {"warehouse", 53, 28},
      {"inn", 55, 28, wares = {pittabread = 4, meat = 4}},
      {"tavern", 57, 28, wares = {pittabread=4, meat = 4}},
      {"well", 52, 30},
      {"farm", 54, 33},
      {"bakery", 51, 35, wares = {wheat = 6, water = 6}},
      {"well", 52, 37}
   )

   -- Adjust the borders so that the village owns everything green
   p:conquer(wl.map.Field(59, 16), 2)
   p:conquer(wl.map.Field(57, 18), 2)
   p:conquer(wl.map.Field(58, 19), 1)
   p:conquer(wl.map.Field(58, 20), 1)
   p:conquer(wl.map.Field(54, 15), 1)
   p:conquer(wl.map.Field(54, 16), 1)
   p:conquer(wl.map.Field(54, 20), 1)
   p:conquer(wl.map.Field(54, 22), 1)
   p:conquer(wl.map.Field(57, 23), 1)
   p:conquer(wl.map.Field(58, 24), 1)
   p:conquer(wl.map.Field(57, 27), 1)
   p:conquer(wl.map.Field(56, 31), 1)
   p:conquer(wl.map.Field(56, 33), 1)
   p:conquer(wl.map.Field(52, 32), 1)

   -- Build roads
   -- Start at northern sentry
   connected_road(p, wl.map.Field(58, 10).immovable,
      "w,sw|se,sw|e,se|se,se|sw,sw|sw,w|sw,sw|se,sw|sw,sw|se,sw|" ..
      "sw,sw|sw,sw|sw,sw|se,se,sw|e,e|sw,sw|se,sw|")

   connected_road(p, wl.map.Field(57, 25).immovable, "sw,w|sw,w")
   connected_road(p, wl.map.Field(57, 29).immovable, "w,w|w,w")
   connected_road(p, wl.map.Field(55, 34).immovable, "sw,sw")
   connected_road(p, wl.map.Field(57, 22).immovable, "sw,w")
   connected_road(p, wl.map.Field(54, 19).immovable, "sw,se,e")
   connected_road(p, wl.map.Field(56, 17).immovable, "sw,se")
end

run(introduction_thread)
run(mines_and_food_thread)
run(build_materials_thread)
run(story_messages_thread)
run(village_thread)

run(mission_complete_thread)


