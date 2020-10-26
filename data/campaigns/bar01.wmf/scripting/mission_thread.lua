-- =======================================================================
--                            Main mission thread
-- =======================================================================

quarry_done = false
enhance_buildings_done = false
build_materials_done = false
cattle_farm_done = false

-- =====================================================
-- First messages to the player at beginning of mission
-- =====================================================

function introduction_thread()
   reveal_concentric(plr, sf, 13)
   sleep(2000)

   campaign_message_box(briefing_msg_01)
   -- these buildings are still burning, but only for a while
   map:place_immovable("destroyed_building", map:get_field(7,41))
   map:place_immovable("destroyed_building", map:get_field(5,52))
   scroll_to_field(al_thunran)
   reveal_concentric(plr, al_thunran, 8, true, 50)
   campaign_message_box(briefing_msg_02) -- Al'thunran
   scroll_to_field(sf)
   sleep(1000)
   scroll_to_field(grave)
   reveal_concentric(plr, grave, 4)
   campaign_message_box(briefing_msg_03) -- grave, Boldreth
   campaign_message_box(briefing_msg_04) -- wait
   -- introduction of Khantrukh
   campaign_message_box(briefing_msg_08)

   campaign_message_box(order_msg_ranger)
   local obj = add_campaign_objective(obj_build_rangers)

   -- Try not to interrupt the player actions with a message, so we wait generously
   sleep(50000)
   campaign_message_box(briefing_msg_05) -- war goes on
   campaign_message_box(briefing_msg_06) -- brothers brought men
   sleep(50000)
   campaign_message_box(briefing_msg_07) -- still living in huts and barracks
   sleep(50000)
   campaign_message_box(msg_story_2) -- winter is upon us
   sleep(10000)

   while not check_for_buildings(plr, {barbarians_rangers_hut = 2}) do sleep(500) end
   set_objective_done(obj)

   plr:allow_buildings{"barbarians_sentry", "barbarians_barrier"}

   campaign_message_box(order_msg_1) -- Boldreth, swords rusty
   campaign_message_box(order_msg_2) -- Kantrukh, rocks

   -- Reveal the rocks
   local rocks = wl.Game().map:get_field(27, 48)
   local prior_center = scroll_to_field(rocks)
   reveal_concentric(plr, rocks, 5)
   campaign_message_box(order_msg_3)
   obj = add_campaign_objective(obj_claim_northeastern_rocks)
   campaign_message_box(order_msg_4)

   -- Move back
   scroll_to_map_pixel(prior_center)

   sleep(50000)
   campaign_message_box(msg_story_1)

   -- Now, wait till the quarry comes up
   local f = wl.Game().map:get_field(27,48):region(6)
   while not check_for_buildings(plr, { barbarians_quarry = 1 }, f) do
      sleep(5000)
   end
   set_objective_done(obj)

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

   -- Send a message and add the objective
   campaign_message_box(order_msg_6_geologist)
   o = add_campaign_objective(obj_build_mines)
   plr:allow_buildings{
      "barbarians_coalmine",
      "barbarians_ironmine",
      "barbarians_goldmine",
      "barbarians_granitemine"
   }

   sleep(10000)
   campaign_message_box(story_msg2)

   -- Wait for completion
   while not check_for_buildings(plr, {barbarians_coalmine = 1, barbarians_ironmine = 1}) do
      sleep(5000)
   end
   set_objective_done(o)
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
            "barbarians_hunters_hut",
            "barbarians_gamekeepers_hut",
            "barbarians_tavern"
         }
         if #rv.barbarians_hunters_hut >= 1 and not hunter_msg_done then
            campaign_message_box(order_msg_11_basic_food_began)
            hunter_msg_done = true
         end
         if #rv.barbarians_tavern >= 1 and not tavern_msg_done then
            campaign_message_box(order_msg_13_tavern)
            tavern_msg_done = true
         end
         if #rv.barbarians_hunters_hut >= 1 and #rv.barbarians_gamekeepers_hut >= 1
                  and #rv.barbarians_tavern >= 1 then break end
         sleep(5331)
      end
      set_objective_done(obj_bf)
      campaign_message_box(story_msg4)
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
      campaign_message_box(order_msg_12_farming_began)
      set_objective_done(obj_farming)
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
   campaign_message_box(order_msg_14_refine_ore)
   plr:allow_buildings{"barbarians_smelting_works"}
   o = add_campaign_objective(obj_refine_ores)
   while #plr:get_buildings("barbarians_smelting_works") < 1 do
      sleep(6223)
   end
   set_objective_done(o)

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
   campaign_message_box(order_msg_15_mines_exhausted)
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
      set_objective_done(obj)
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
   sleep(5000)

   campaign_message_box(order_msg_16_blackwood)
   plr:allow_buildings{"barbarians_wood_hardener"}
   sf.immovable:set_wares("blackwood",5)
   -- So that player has really little, but still enough to expand a bit
   local o = add_campaign_objective(obj_better_material_1)

   sleep(50000)
   campaign_message_box(story_msg1)
   while #plr:get_buildings("barbarians_wood_hardener") < 1 do sleep(5421) end
   set_objective_done(o)

   campaign_message_box(order_msg_17_grout)
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
   set_objective_done(o)

   campaign_message_box(order_msg_18_reed)
   plr:allow_buildings{"barbarians_reed_yard"}
   o = add_campaign_objective(obj_better_material_3)
   while #plr:get_buildings("barbarians_reed_yard") < 1 do sleep(5421) end

   campaign_message_box(order_msg_19_all_material)
   set_objective_done(o)

   build_materials_done = true
end

-- ==================
-- Cattlefarm thread
-- ==================
function cattle_farm()
   while not check_for_buildings(plr, { barbarians_farm = 1, barbarians_well = 1 }) do
      sleep(7834)
   end

   campaign_message_box(msg_cattlefarm_00)

   local o = add_campaign_objective(obj_build_cattlefarm)
   plr:allow_buildings{"barbarians_cattlefarm"}

   sleep(10000)
   campaign_message_box(story_msg3)

   while not check_for_buildings(plr, { barbarians_cattlefarm = 1 }) do
      sleep(2323)
   end
   set_objective_done(o)

   cattle_farm_done = true
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
   plr:mark_scenario_as_solved("bar01.wmf")
end


run(introduction_thread)
run(mines_and_food_thread)
run(build_materials_thread)

include "map:scripting/secret_village.lua" -- starts the thread

run(mission_complete_thread)
