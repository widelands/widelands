function mission_thread()
   sleep(1000)

   -- Initial messages
   local sea = wl.Game().map:get_field(50,25)
   local ship = p1:place_ship(sea)
   p1:hide_fields(sea:region(6),true)
   scroll_to_field(sea,0)

   campaign_message_box(diary_page_1)
   sleep(200)

   -- Show the sea
   reveal_concentric(p1, sea, 5)
   sleep(1000)
   campaign_message_box(diary_page_2)
   sleep(500)
   hide_concentric(p1, sea, 5)
   ship:remove()

   -- Back home
   include "map:scripting/starting_conditions.lua"
   p1:hide_fields(wl.Game().map.player_slots[1].starting_field:region(13),true)
   scroll_to_field(wl.Game().map.player_slots[1].starting_field)
   campaign_message_box(diary_page_3)
   sleep(1000)
   reveal_concentric(p1, wl.Game().map.player_slots[1].starting_field, 13)
   sleep(400)

   -- Check for trees and remove them
   local fields = {{12,0},          -- Buildspace
                   {12,1},          -- Flag of building
                   {12,2}, {11,2},  -- Roads ...
                   {10,2}, {9,2},
                   {8,2}, {7,1},
                   {7,0},}
   remove_trees(fields)

   campaign_message_box(saledus_1)
   p1:allow_buildings{"empire_blockhouse"}
   local o = add_campaign_objective(obj_build_blockhouse)
   while #p1:get_buildings("empire_blockhouse") < 1 do sleep(3249) end
   set_objective_done(o)

   -- Blockhouse is completed now
   -- Make sure no tree blocks the building space for Lumberjack
   local fields = {{6,3},           -- Buildspace
                   {7,4},           -- Flag of building
                   {7,3}, {7,2},}   -- Roads
   remove_trees(fields)

   campaign_message_box(saledus_2)
   p1:allow_buildings{"empire_lumberjacks_house"}
   o = add_campaign_objective(obj_build_lumberjack)
   campaign_message_box(amalea_1)
   while #p1:get_buildings("empire_lumberjacks_house") < 1 do sleep(3249) end
   set_objective_done(o)

   p1:mark_training_wheel_as_solved("logs")

   -- Lumberjack is now build
   campaign_message_box(amalea_2)
   p1:allow_buildings{"empire_sawmill"}
   o = add_campaign_objective(obj_build_sawmill_and_lumberjacks)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 3, empire_sawmill = 1})
      do sleep(2343) end
   set_objective_done(o)

   -- Now the lady demands a forester after having us cut down the whole forest.
   campaign_message_box(amalea_3)
   o = add_campaign_objective(obj_build_forester)
   p1:allow_buildings{"empire_foresters_house"}
   while not check_for_buildings(p1, { empire_foresters_house = 1 }) do sleep(2434) end
   set_objective_done(o)


   -- Now a quarry
   campaign_message_box(saledus_3)
   o = add_campaign_objective(obj_build_quarry)
   p1:allow_buildings{"empire_quarry"}
   while not check_for_buildings(p1, { empire_quarry = 1 }) do sleep(2434) end
   set_objective_done(o)
   p1:mark_training_wheel_as_solved("rocks")

   -- All buildings done. Got home
   campaign_message_box(saledus_4)

   sleep(25000) -- Sleep a while

   campaign_message_box(diary_page_4)
   p1:mark_scenario_as_solved("emp01.wmf")
end

-- Show a funny message when the player has build 10 blockhouses
function easter_egg()
   while not check_for_buildings(p1, {empire_blockhouse = 10}) do sleep(4253) end

   campaign_message_box(safe_peninsula)
end


run(mission_thread)
run(easter_egg)
