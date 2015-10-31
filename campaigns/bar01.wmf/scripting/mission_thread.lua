-- =======================================================================
--                            Main mission thread
-- =======================================================================

include "scripting/table.lua"
include "scripting/ui.lua"

quarry_done = false
enhance_buildings_done = false
build_materials_done = false
cattle_farm_done = false

-- =====================================================
-- First messages to the player at beginning of mission
-- =====================================================

function introduction_thread()
   sleep(2000)

   message_box_objective(plr, briefing_msg_01)
   -- these buildings are still burning, but only for a while
   map:place_immovable("destroyed_building",map:get_field(7,41),"tribes")
   map:place_immovable("destroyed_building",map:get_field(5,52),"tribes")
   plr:reveal_fields(al_thunran:region(8))
   message_box_objective(plr, briefing_msg_02) -- Al'thunran
   plr:reveal_fields(grave:region(4))
   message_box_objective(plr, briefing_msg_03) -- grave
   message_box_objective(plr, briefing_msg_04)

   message_box_objective(plr, briefing_msg_05)
   message_box_objective(plr, briefing_msg_06)
   message_box_objective(plr, briefing_msg_07)

   -- introduction of Khantrukh
   message_box_objective(plr, briefing_msg_08)

   message_box_objective(plr, order_msg_ranger)
   local obj = add_campaign_objective(obj_build_rangers)

   while not check_for_buildings(plr, {barbarians_rangers_hut = 2}) do sleep(500) end
   obj.done = true

   plr:allow_buildings{"barbarians_sentry", "barbarians_barrier"}

   message_box_objective(plr, order_msg_1)
   message_box_objective(plr, order_msg_2)

   -- Reveal the rocks
   local rocks = wl.Game().map:get_field(27, 48)
   plr:reveal_fields(rocks:region(6))
   local way = scroll_smoothly_nordwards_to(rocks, 1500)

   message_box_objective(plr, order_msg_3)
   obj = add_campaign_objective(obj_claim_northeastern_rocks)
   message_box_objective(plr, order_msg_4)

   -- Move back
   timed_scroll(array_reverse(way), 10)

   -- Now, wait till the quarry comes up
   local f = wl.Game().map:get_field(27,48):region(6)
   while not check_for_buildings(plr, { barbarians_quarry = 1 }, f) do
      sleep(5000)
   end
   obj.done = true

   message_box_objective(plr, order_msg_5_quarry)

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

   -- Send a message and add the objective
   message_box_objective(plr, order_msg_6_geologist)
   o = add_campaign_objective(obj_build_mines)
   plr:allow_buildings{
      "barbarians_coalmine",
      "barbarians_ironmine",
      "barbarians_goldmine",
      "barbarians_granitemine"
   }

   sleep(10000)
   message_box_objective(plr, story_msg2)

   -- Wait for completion
   while not check_for_buildings(plr, {barbarians_coalmine = 1, barbarians_ironmine = 1}) do
      sleep(5000)
   end
   o.done = true
   message_box_objective(plr, order_msg_7_mines_up)
   message_box_objective(plr, order_msg_8_mines_up)
   message_box_objective(plr, order_msg_9_hunter)
   message_box_objective(plr, order_msg_10_bread)

   local obj_bf = add_campaign_objective(obj_basic_food)
   -- The function to check for completeness
   run(function()
      local tavern_msg_done = nil
      local hunter_msg_done = nil
      while true do
         local rv = plr:get_buildings{
            "barbarians_hunters_hut",
            "barbarians_gamekeepers_hut",
            "barbarians_tavern"
         }
         if #rv.barbarians_hunters_hut >= 1 and not hunter_msg_done then
            message_box_objective(plr, order_msg_11_basic_food_began)
            hunter_msg_done = true
         end
         if #rv.barbarians_tavern >= 1 and not tavern_msg_done then
            message_box_objective(plr, order_msg_13_tavern)
            tavern_msg_done = true
         end
         if #rv.barbarians_hunters_hut >= 1 and #rv.barbarians_gamekeepers_hut >= 1
                  and #rv.barbarians_tavern >= 1 then break end
         sleep(5331)
      end
      obj_bf.done = true
      sleep(5000)
      message_box_objective(plr, story_msg4)
   end)

   local obj_farming = add_campaign_objective(obj_begin_farming)

   -- Start the cattlefarm thread
   run(cattle_farm)

   run(function()
      while 1 do
         local rv = plr:get_buildings{"barbarians_well", "barbarians_bakery", "barbarians_farm"}
         if #rv.barbarians_well >= 1 and #rv.barbarians_bakery >= 1 and #rv.barbarians_farm >= 1 then
            break
         end
         sleep(4234)
      end
      message_box_objective(plr, order_msg_12_farming_began)
      obj_farming.done = true
   end)

   -- Enable food production
   plr:allow_buildings{
      "barbarians_hunters_hut",
      "barbarians_fishers_hut",
      "barbarians_gamekeepers_hut",
      "barbarians_tavern",
      "barbarians_farm",
      "barbarians_well",
      "barbarians_bakery",
   }

   while not (obj_bf.done and obj_farming.done) do
      sleep(6231)
   end

   -- Ready to build refiner stuff
   message_box_objective(plr, order_msg_14_refine_ore)
   plr:allow_buildings{"barbarians_smelting_works"}
   o = add_campaign_objective(obj_refine_ores)
   while #plr:get_buildings("barbarians_smelting_works") < 1 do
      sleep(6223)
   end
   o.done = true

   -- Information about making mines deeper
   -- Wait until the player has an experienced worker.
   local chiefminer_found = false
   while not chiefminer_found do
      local mines = plr:get_buildings{"barbarians_coalmine","barbarians_ironmine"}
      for k,v in ipairs(mines.barbarians_coalmine) do
         if v:get_workers("barbarians_miner_chief") > 0 then
            chiefminer_found = true
            break
         end
      end
      for k,v in ipairs(mines.barbarians_ironmine) do
         if v:get_workers("barbarians_miner_chief") > 0 then
            chiefminer_found = true
            break
         end
      end

      -- If everything else is done, show this objective, too. Otherwise, the player has nothing to do.
      if (build_materials_done and quarry_done and cattle_farm_done) then
         chiefminer_found = true
      end
      sleep(5000)
   end
   message_box_objective(plr, order_msg_15_mines_exhausted)
   plr:allow_buildings{
      "barbarians_coalmine_deep",
      "barbarians_ironmine_deep",
      "barbarians_inn",
      "barbarians_micro_brewery"}
   local obj = add_campaign_objective(obj_enhance_buildings)

   run(function()
      while true do
         local rv = plr:get_buildings{
            "barbarians_coalmine_deep",
            "barbarians_ironmine_deep",
            "barbarians_inn",
            "barbarians_micro_brewery"}
         if (#rv.barbarians_inn > 0 and #rv["barbarians_micro_brewery"] > 0) and
            (#rv.barbarians_coalmine_deep + #rv.barbarians_ironmine_deep > 0) then
            break
         end
         sleep(5421)
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

   -- Wait for a barrier or sentry to be built
   while true do
      local rv = plr:get_buildings{"barbarians_sentry", "barbarians_barrier"}
      if #rv.barbarians_sentry + #rv.barbarians_barrier > 0 then
         break
      end
      sleep(5421)
   end

   message_box_objective(plr, order_msg_16_blackwood)
   plr:allow_buildings{"barbarians_wood_hardener"}
   sf.immovable:set_wares("blackwood",5)
   -- So that player has really little, but still enough to expand a bit
   local o = add_campaign_objective(obj_better_material_1)

   sleep(30*1000)
   message_box_objective(plr, story_msg1)
   while #plr:get_buildings("barbarians_wood_hardener") < 1 do sleep(5421) end
   o.done = true

   message_box_objective(plr, order_msg_17_grout)
   plr:allow_buildings{"barbarians_lime_kiln", "barbarians_well", "barbarians_charcoal_kiln"}
   o = add_campaign_objective(obj_better_material_2)
   -- Wait for the buildings to be built
   while true do
      local rv = plr:get_buildings{
         "barbarians_lime_kiln",
         "barbarians_well",
         "barbarians_coalmine",
         "barbarians_coalmine_deep",
         "barbarians_charcoal_kiln"}
      if (#rv.barbarians_lime_kiln > 0 and #rv.barbarians_well > 0) and
         (#rv.barbarians_coalmine + #rv.barbarians_coalmine_deep + #rv.barbarians_charcoal_kiln > 0) then
         break
      end
      sleep(5421)
   end
   o.done = true

   message_box_objective(plr, order_msg_18_reed)
   plr:allow_buildings{"barbarians_reed_yard"}
   o = add_campaign_objective(obj_better_material_3)
   while #plr:get_buildings("barbarians_reed_yard") < 1 do sleep(5421) end

   message_box_objective(plr, order_msg_19_all_material)
   o.done = true

   build_materials_done = true
end

-- ==================
-- Cattlefarm thread
-- ==================
function cattle_farm()
   while not check_for_buildings(plr, { barbarians_farm = 1, barbarians_well = 1 }) do
      sleep(7834)
   end

   message_box_objective(plr, msg_cattlefarm_00)

   local o = add_campaign_objective(obj_build_cattlefarm)
   plr:allow_buildings{"barbarians_cattlefarm"}

   sleep(10000)
   message_box_objective(plr, story_msg3)

   while not check_for_buildings(plr, { barbarians_cattlefarm = 1 }) do
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
   message_box_objective(plr, msg_story_2)

   wake_me(600 * 1000)
   message_box_objective(plr, msg_story_1)
end

-- =================
-- Mission complete
-- =================
function mission_complete_thread()
   while not (build_materials_done and quarry_done
            and enhance_buildings_done and cattle_farm_done) do
         sleep(10000)
   end

   message_box_objective(plr, msg_mission_complete)
   plr:reveal_scenario("barbariantut01")
end


run(introduction_thread)
run(mines_and_food_thread)
run(build_materials_thread)
run(story_messages_thread)

include "map:scripting/secret_village.lua" -- starts the thread

run(mission_complete_thread)