include "scripting/messages.lua"

function mission_thread()
   
   --Introduction
   scroll_to_field(wl.Game().map.player_slots[1].starting_field)
   campaign_message_box(intro_1)
   sleep(1000)
   include "map:scripting/starting_conditions.lua"
   sleep(2000)
   
   --Objective: Basic material economy
   campaign_message_box(intro_2)
   local o = add_campaign_objective(obj_new_home)
   while not check_for_buildings(p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 1, frisians_well = 1, frisians_reed_farm = 1, frisians_quarry = 1, frisians_brick_burners_house = 1, frisians_claypit = 1}) do sleep(4273) end
   set_objective_done(o)

   --Objective: Go South
   campaign_message_box(expand_s_1)
   local o = add_campaign_objective(obj_expand_south)
   while not  do sleep(4273) end
   set_objective_done(o)

   
   
   
   
   while true do sleep(10001) end
   sleep(1000)
   campaign_message_box(victory_1)
   p1:reveal_scenario("frisians02")
end

run(mission_thread)
