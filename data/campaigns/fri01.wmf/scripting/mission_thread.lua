include "scripting/messages.lua"

function mission_thread()
   sleep(1000)
   campaign_message_box(intro_1)

   include "map:scripting/starting_conditions.lua"
   scroll_to_field(wl.Game().map.player_slots[1].starting_field)
   sleep(1000)
   campaign_message_box(intro_2)
   p1:allow_buildings{"frisians_woodcutters_house", "frisians_foresters_house", "frisians_reed_farm"}
   local o = add_campaign_objective(obj_build_wood_economy)
   while not check_for_buildings(p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 2, frisians_reed_farm = 1}) do sleep(4273) end
   set_objective_done(o)

   campaign_message_box(intro_3)
   p1:allow_buildings{"frisians_brick_burners_house", "frisians_well", "frisians_claypit", "frisians_coalmine", "frisians_rockmine"}
   o = add_campaign_objective(obj_build_brick_economy)
   while not check_for_buildings(p1, {frisians_brick_burners_house = 1, frisians_well = 1, frisians_claypit = 1, frisians_coalmine = 1, frisians_rockmine = 1}) do sleep(4273) end
   set_objective_done(o)
   campaign_message_box(intro_4)
   
   --TODO wait until no rations are left in the whole economy
   local rationsLeft = 1
   while rationsLeft > 0 do
      local whs = array_combine(
         p1:get_buildings("frisians_headquarters"),
         p1:get_buildings("frisians_warehouse"),
         p1:get_buildings("frisians_coalmine"),
         p1:get_buildings("frisians_rockmine")
      )
      rationsLeft = 0
      for idx,wh in ipairs(whs) do
         rationsLeft = rationsLeft + wh:get_wares("ration")
      end
      sleep(4273)
   end
   
   campaign_message_box(food_1)
   p1:allow_buildings{"frisians_fishers_house", "frisians_smokery", "frisians_hunters_house", "frisians_tavern", "frisians_berry_farm", "frisians_collectors_house"}
   o = add_campaign_objective(obj_build_food_economy)
   while not check_for_buildings(p1, {frisians_fishers_house = 1, frisians_smokery = 1, frisians_tavern = 1, frisians_berry_farm = 1, frisians_collectors_house = 1}) do sleep(4273) end
   set_objective_done(o)
   campaign_message_box(food_2)
   p1:allow_buildings{"frisians_beekeepers_house", "frisians_drinking_hall", "frisians_farm", "frisians_bakery", "frisians_brewery", "frisians_mead_brewery", "frisians_honey_bread_bakery"}
   o = add_campaign_objective(obj_build_food_economy_2)
   while not check_for_buildings(p1, {frisians_farm = 2, frisians_bakery = 1, frisians_tavern = 1, frisians_brewery = 1, frisians_drinking_hall = 1, frisians_beekeepers_house = 1}) do sleep(4273) end
   set_objective_done(o)
   campaign_message_box(mining_1)
   p1:allow_buildings{"frisians_ironmine", "frisians_goldmine", "frisians_coalmine_deep", "frisians_rockmine_deep", "frisians_ironmine_deep", "frisians_goldmine_deep", "frisians_furnace", "frisians_blacksmithy", "frisians_armour_smithy_small"}
   o = add_campaign_objective(obj_build_mining)
   while not check_for_buildings(p1, {frisians_ironmine = 1, frisians_goldmine = 1, frisians_furnace = 1, frisians_blacksmithy = 1, frisians_armour_smithy_small = 1}) do sleep(4273) end
   set_objective_done(o)

   --TODO wait until enemy sighted
   
   campaign_message_box(recruiting_1)
   p1:reveal_fields(wl.Game().map.player_slots[2].starting_field:region(6))
   scroll_to_field(wl.Game().map.player_slots[2].starting_field)
   campaign_message_box(recruiting_2)
   scroll_to_field(wl.Game().map.player_slots[1].starting_field) --TODO scroll instead back to where we were before scrolling to enemy HQ
   campaign_message_box(recruiting_3)
   p1:hide_fields(wl.Game().map.player_slots[2].starting_field:region(6))
   sleep(2000)
   campaign_message_box(recruiting_4)
   p1:allow_buildings{"frisians_barracks", "frisians_reindeer_farm", "frisians_seamstress"}
   o = add_campaign_objective(obj_recruit_soldiers)
   while not check_for_buildings(p1, {frisians_barracks = 1, frisians_seamstress = 1, frisians_reindeer_farm = 1}) do sleep(4273) end
   set_objective_done(o)
   
   campaign_message_box(training_1)
   p1:allow_buildings{"frisians_training_camp", "frisians_training_arena", "frisians_seamstress_master", "frisians_armour_smithy_large"}
   o = add_campaign_objective(obj_train_soldiers)
   --TODO wait until at least 1 soldier has level 10
   set_objective_done(o)
   campaign_message_box(training_2)
   p1:allow_buildings("all")
   p1:forbid_buildings{"frisians_port", "frisians_shipyard", "frisians_weaving_mill"}
   o = add_campaign_objective(obj_defeat_enemy)
   while not p2:defeated() do sleep(4273) end
   set_objective_done(o)
   
   scroll_to_field(wl.Game().map.player_slots[2].starting_field)
   campaign_message_box(rising_water_1)
   campaign_message_box(rising_water_2)
   --TODO scroll to the westernmost field and flood it
   campaign_message_box(rising_water_3)
   campaign_message_box(rising_water_4)
   p1:allow_buildings("all")
   o = add_campaign_objective(obj_escape)
   
   --TODO wait until an expedition ship is waiting
   
   sleep(10000) -- Sleep a while
   campaign_message_box(victory_1)
   p1:reveal_scenario("frisians01")
end

run(mission_thread)
