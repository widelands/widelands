-- =======================================================================
--                              Mission Threads
-- =======================================================================

function building_materials()
   reveal_concentric(p1, wl.Game().map.player_slots[1].starting_field, 13)
   sleep(1000)
   campaign_message_box(diary_page_5)

   local map = wl.Game().map
   local forests = map:get_field(17,10)
   p1:reveal_fields(forests:region(6))
   -- Hide again in 5 seconds
   run(function() sleep(5000) p1:hide_fields(forests:region(6)) end)

   -- Show the trees
   wait_for_roadbuilding_and_scroll(forests)

   campaign_message_box(diary_page_5_1)
   local o_woodeconomy = add_campaign_objective(obj_build_woodeconomy)
   -- Check for completing of the woodeconomy
   run(function()
      while not check_for_buildings(p1, {
         empire_lumberjacks_house = 3,
         empire_foresters_house = 1,
         empire_sawmill = 1,
      }) do sleep(2342) end

      campaign_message_box(diary_page_6)
      set_objective_done(o_woodeconomy)
   end)


   -- Show the stones
   wait_for_roadbuilding_and_scroll(map:get_field(14,25))
   campaign_message_box(diary_page_5_2)
   local o_quarry = add_campaign_objective(obj_build_quarry)
   -- Check for completeness of the quarry
   run(function()
      while not check_for_buildings(p1, {empire_quarry=1}) do sleep(3423) end
      set_objective_done(o_quarry)

      campaign_message_box(diary_page_7)
   end)
end

function food_thread()
   -- Wait till one mine goes up
   while true do
      local rv = p1:get_buildings{
         "empire_coalmine",
         "empire_coalmine_deep",
         "empire_ironmine",
         "empire_ironmine_deep",
         "empire_marblemine",
      }
      local s = 0
      for idx,a in pairs(rv) do s = s + #a end
      if s > 0 then break end
      sleep(2344)
   end

   campaign_message_box(amalea_1)
   p1:allow_buildings{
      "empire_well",
      "empire_farm",
      "empire_brewery",
      "empire_vineyard",
      "empire_winery",
      "empire_mill",
      "empire_bakery",
      "empire_hunters_house",
      "empire_piggery",
      "empire_tavern",
      "empire_inn",
      "empire_fishers_house",
   }
   local o = add_campaign_objective(obj_build_food_infrastructure)

   -- Run easter egg: Amalea says something when Tavern is done
   run(function()
      while #p1:get_buildings("empire_tavern") < 1 do sleep(2349) end
      campaign_message_box(amalea_2)
   end)

   while true do
      local rv = p1:get_buildings{
         "empire_brewery",
         "empire_fishers_house",
         "empire_bakery",
         "empire_hunters_house",
         "empire_piggery",
         "empire_tavern",
         "empire_inn",
      }
      if #rv.empire_brewery > 0 and
         (#rv.empire_tavern + #rv.empire_inn) > 0 and
         (#rv.empire_fishers_house + #rv.empire_hunters_house +
          #rv.empire_piggery + #rv.empire_bakery) > 0 then break end
      sleep(4857)
   end
   set_objective_done(o)
end

local build_marble_mine_done = false
function mining_infrastructure()
   -- Wait for marble mine
   while not (build_marble_mine_done) do sleep(3459) end

   -- Reveal the other mountains
   local coal_mountain = wl.Game().map:get_field(49,22)
   local iron_mountain = wl.Game().map:get_field(38,37)

   wait_for_roadbuilding_and_scroll(coal_mountain)
   reveal_concentric(p1, coal_mountain, 6, false)
   p1:reveal_fields(iron_mountain:region(6))
   run(function() sleep(5000)
      p1:hide_fields(coal_mountain:region(6))
      p1:hide_fields(iron_mountain:region(6))
   end)

   campaign_message_box(saledus_3)
   p1:allow_buildings{
      "empire_coalmine",
      "empire_coalmine_deep",
      "empire_ironmine",
      "empire_ironmine_deep",
      "empire_stonemasons_house",
      "empire_toolsmithy",
      "empire_armorsmithy",
      "empire_weaponsmithy",
      "empire_smelting_works",
      "empire_charcoal_kiln",
   }

   local o = add_campaign_objective(obj_build_mining_infrastructure)
   -- Wait for the infrastructure to come up
   while true do
      local rv = p1:get_buildings{
         "empire_coalmine",
         "empire_coalmine_deep",
         "empire_ironmine",
         "empire_ironmine_deep",
         "empire_stonemasons_house",
         "empire_toolsmithy",
         "empire_armorsmithy",
         "empire_weaponsmithy",
         "empire_smelting_works",
         "empire_charcoal_kiln",
      }
      if #rv.empire_stonemasons_house > 0 and
         #rv.empire_toolsmithy > 0 and
         #rv.empire_armorsmithy > 0 and
         #rv.empire_smelting_works > 0 and
         (#rv.empire_ironmine + #rv.empire_ironmine_deep > 0) and
         (#rv.empire_coalmine + #rv.empire_coalmine_deep + #rv.empire_charcoal_kiln > 0) and
         #rv.empire_weaponsmithy > 0 then break end
      sleep(4948)
   end
   set_objective_done(o)
   mining_infrastructure_done = true
end

function expand_and_build_marblemine()
   sleep(40000)

   -- Move to the shipparts
   local shipparts = wl.Game().map:get_field(15,46)
   local prior_center = wait_for_roadbuilding_and_scroll(wl.Game().map:get_field(12,43))
   reveal_concentric(p1, shipparts, 5)
   run(function() sleep(10000) p1:hide_fields(shipparts:region(5)) end)
   campaign_message_box(saledus_1)
   local o = add_campaign_objective(obj_build_military_buildings)
   p1:allow_buildings{"empire_blockhouse", "empire_sentry"}

   -- Go back to where we were
   scroll_to_map_pixel(prior_center)

   -- sleep while not owning 26, 21
   while wl.Game().map:get_field(26,21).owner ~= p1 do sleep(3243) end
   set_objective_done(o)

   -- Marble Mountains
   local marblemountains = wl.Game().map:get_field(35,19)

   prior_center = wait_for_roadbuilding_and_scroll(marblemountains)
   reveal_concentric(p1, marblemountains, 5, false)
   run(function() sleep(10000) p1:hide_fields(marblemountains:region(5)) end)

   campaign_message_box(saledus_2)
   p1:allow_buildings{"empire_marblemine", "empire_marblemine_deep"}
   o = add_campaign_objective(obj_build_marblemine)
   run(function() while not check_for_buildings(p1, {empire_marblemine = 1})
      do sleep(2133) end set_objective_done(o, 0); build_marble_mine_done = true end)

   -- Go back to where we were
   scroll_to_map_pixel(prior_center)
end

function barbarians_thread()
   -- Wait for various buildings to go up
   while true do
      local rv = p1:get_buildings{
         "empire_marblemine", "empire_marblemine_deep",
         "empire_ironmine", "empire_ironmine_deep",
         "empire_coalmine", "empire_coalmine_deep", "empire_charcoal_kiln"
      }
      local mm = #rv.empire_marblemine + #rv.empire_marblemine_deep
      local ore = #rv.empire_ironmine + #rv.empire_ironmine_deep
      local coal = #rv.empire_coalmine + #rv.empire_coalmine_deep + #rv.empire_charcoal_kiln

      if mm > 0 and ore > 0 and coal > 0 then break end
      sleep(6674)
   end

   -- show Barbarians
   local barbarians = array_combine(
      wl.Game().map:get_field(60, 61):region(6),
      wl.Game().map:get_field(60, 48):region(6)
   )
   p1:reveal_fields(barbarians)
   p2.hidden_from_general_statistics = false
   run(function() sleep(5000) p1:hide_fields(barbarians) end)
   wait_for_roadbuilding_and_scroll(wl.Game().map:get_field(59, 55))

   campaign_message_box(diary_page_8)
   local o = add_campaign_objective(obj_build_bigger_military_buildings)
   p1:allow_buildings{
      "empire_outpost",
      "empire_barrier",
      "empire_tower",
      "empire_warehouse",
      "empire_donkeyfarm"}
   p2:allow_buildings{"barbarians_quarry"}

   -- Wait for one of the buildings to go up
   while true do
      local rv = p1:get_buildings{"empire_outpost", "empire_barrier", "empire_tower"}
      if (#rv.empire_tower + #rv.empire_outpost + #rv.empire_barrier) > 0 then
         break
      end
      sleep(2342)
   end
   set_objective_done(o)

   -- Wait till the mining infrastructure is also up
   while not mining_infrastructure_done do
      sleep(2343)
   end
   campaign_message_box(diary_page_9)
   p1:allow_buildings{"empire_barracks", "empire_fortress", "empire_sheepfarm", "empire_weaving_mill"}

   o = add_campaign_objective(obj_remove_the_barbarians)
   -- Wait for the fortress to come up
   while not check_for_buildings(p1, {empire_fortress = 1},
      wl.Game().map:get_field(60,65):region(6))
   do sleep(2435) end

   set_objective_done(o)

   -- Show victory message
   campaign_message_box(diary_page_10)
   campaign_message_box(seven_days_later)
   campaign_message_box(diary_page_11)

   p1:mark_scenario_as_solved("emp02.wmf")
end

run(building_materials)
run(food_thread)
run(mining_infrastructure)
run(expand_and_build_marblemine)
run(barbarians_thread)
