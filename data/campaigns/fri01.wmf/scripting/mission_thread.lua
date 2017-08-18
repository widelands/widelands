include "scripting/messages.lua"

function stormflood() --TODO fix memory leaks
   local x
   local y
   local yD
   local up
   local check
   local nextF = firstToFlood
   while nextF do
      --flood field nextF
      nextF.terr = "winter_water"
      nextF.terd = "winter_water"
      --turn the settlement into Rungholt (or Atlantis, if you prefer)
      for idx,field in ipairs(nextF:region(1)) do
         if field.immovable then field.immovable:remove() end
         for idb,bob in ipairs(field.bobs) do bob:remove() end
      end
      --Set nextF to westernmost unflooded field. To ensure waveform, select of all fields here the one closest (y-direction) to floodstart
      x = nextF.x
      nextF = nil
      while nextF == nil and x < wl.Game().map.width do
         yD = 0
         up = true
         while nextF == nil and yD < ( wl.Game().map.height / 2 ) do
            y = firstToFlood.y
            if up then y = y + yD else y = y - yD end
            if y < 0 then y = y + wl.Game().map.height end
            if not ( y < wl.Game().map.height ) then y = y - wl.Game().map.height end
            check = wl.Game().map:get_field(x, y)
            if ( not ( check.terr == "winter_water" ) or not ( check.terd == "winter_water" )) and check.height < 4 then nextF = check end
            if up then yD = yD + 1 end
            up = not up
         end
         x = x + 1
      end
      sleep(197)
   end
end

function mission_thread()
   
   --Introduction
   sleep(1000)
   campaign_message_box(intro_1)
   
   
   
   --TEST TEST TEST
   
   --p1.see_all = true
   --scroll_to_field(firstToFlood)
   --run(stormflood)
   --sleep(3000001)
   
   --END OF TEST
   
   
   
   include "map:scripting/starting_conditions.lua"
   scroll_to_field(wl.Game().map.player_slots[1].starting_field)
   sleep(1000)
   campaign_message_box(intro_2)
   --build wood economy and reed yard
   p1:allow_buildings{"frisians_woodcutters_house", "frisians_foresters_house", "frisians_reed_farm"}
   local o = add_campaign_objective(obj_build_wood_economy)
   while not check_for_buildings(p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 2, frisians_reed_farm = 1}) do sleep(4273) end
   set_objective_done(o)

   campaign_message_box(intro_3)
   --build brick economy
   p1:allow_buildings{"frisians_brick_burners_house", "frisians_well", "frisians_claypit", "frisians_coalmine", "frisians_rockmine"}
   o = add_campaign_objective(obj_build_brick_economy)
   while not check_for_buildings(p1, {frisians_brick_burners_house = 1, frisians_well = 1, frisians_claypit = 1, frisians_coalmine = 1, frisians_rockmine = 1}) do sleep(4273) end
   set_objective_done(o)
   campaign_message_box(intro_4)
   
   --wait until no rations left
   sleep(60000)
   local somethingLeft = true
   while somethingLeft do
      local whs = array_combine(
         p1:get_buildings("frisians_headquarters"),
         p1:get_buildings("frisians_warehouse")
      )
      local inWH = 0
      for idx,wh in ipairs(whs) do
         inWH = inWH + wh:get_wares("ration")
      end
      whs = array_combine(
         p1:get_buildings("frisians_coalmine"),
         p1:get_buildings("frisians_rockmine")
      )
      local mW = false -- has at least 1 mine no rations left?
      for idx,wh in ipairs(whs) do
         mW = mW or ( wh:get_inputs("ration") == 0 )
      end
      somethingLeft = ( inWH > 0 ) or not mW
      sleep(14271)
   end
   
   --great, you forgot to provide rations…
   campaign_message_box(food_1)
   p1:allow_buildings{"frisians_fishers_house", "frisians_smokery", "frisians_hunters_house", "frisians_tavern", "frisians_berry_farm", "frisians_collectors_house"}
   o = add_campaign_objective(obj_build_food_economy)
   while not check_for_buildings(p1, {frisians_fishers_house = 1, frisians_smokery = 1, frisians_tavern = 1, frisians_berry_farm = 1, frisians_collectors_house = 1}) do sleep(4273) end
   set_objective_done(o)

   --we want better food
   campaign_message_box(food_2)
   p1:allow_buildings{"frisians_beekeepers_house", "frisians_farm", "frisians_bakery", "frisians_brewery", "frisians_mead_brewery", "frisians_honey_bread_bakery"}
   o = add_campaign_objective(obj_build_food_economy_2)
   while not check_for_buildings(p1, {frisians_tavern = 2}) do sleep(4273) end
   p1:allow_buildings{"frisians_drinking_hall"}
   while not check_for_buildings(p1, {frisians_farm = 2, frisians_bakery = 1, frisians_brewery = 1, frisians_beekeepers_house = 1}) do sleep(4273) end
   set_objective_done(o)

   --we can start some real mining
   campaign_message_box(mining_1)
   p1:allow_buildings{"frisians_ironmine", "frisians_goldmine", "frisians_coalmine_deep", "frisians_rockmine_deep", "frisians_ironmine_deep", "frisians_goldmine_deep", "frisians_furnace", "frisians_blacksmithy", "frisians_armour_smithy_small"}
   o = add_campaign_objective(obj_build_mining)
   while not check_for_buildings(p1, {frisians_ironmine = 1, frisians_goldmine = 1, frisians_furnace = 1, frisians_blacksmithy = 1, frisians_armour_smithy_small = 1}) do sleep(4273) end
   set_objective_done(o)
   
   p1:allow_buildings{"frisians_outpost"}
   --show the "expand" objective only if we haven´t expanded that far yet
   local skip = not ( expansionMark.owner == nil )
   if not skip then
      campaign_message_box(expand_1)
      o = add_campaign_objective(obj_expand)
   end
   
   --wait until we have conquered the left half of the island
   while expansionMark.owner == nil do sleep(4273) end
   if not skip then set_objective_done(o) end
   
   --a friendly chat between neighbours
   p1:reveal_fields(wl.Game().map.player_slots[2].starting_field:region(6))
   scroll_to_field(wl.Game().map.player_slots[2].starting_field)
   campaign_message_box(recruiting_1)
   campaign_message_box(recruiting_2)
   campaign_message_box(recruiting_3)
   scroll_to_field(expansionMark)
   p1:hide_fields(wl.Game().map.player_slots[2].starting_field:region(6))
   sleep(1000)

   --start recruiting
   campaign_message_box(recruiting_4)
   p1:allow_buildings{"frisians_barracks", "frisians_reindeer_farm", "frisians_seamstress", "frisians_wooden_tower", "frisians_wooden_tower_high"}
   o = add_campaign_objective(obj_recruit_soldiers)
   while not check_for_buildings(p1, {frisians_barracks = 1, frisians_seamstress = 1, frisians_reindeer_farm = 1}) do sleep(4273) end
   set_objective_done(o)
   sleep(30000)

   --start training
   campaign_message_box(training_1)
   p1:allow_buildings{"frisians_training_camp", "frisians_training_arena", "frisians_seamstress_master", "frisians_armour_smithy_large"}
   o = add_campaign_objective(obj_train_soldiers)
   
   --wait until at least 1 soldier has level 10
   local hasL10 = false
   while not hasL10 do
      local milbuild = array_combine(
         p1:get_buildings("frisians_sentinel"),
         p1:get_buildings("frisians_wooden_tower"),
         p1:get_buildings("frisians_wooden_tower_high"),
         p1:get_buildings("frisians_outpost")
      )
      for idx,site in ipairs(milbuild) do
         hasL10 = hasL10 or ( site:get_soldiers{2,6,2,0} > 0 )
      end
      sleep(4273)
   end
   
   --Attack!
   set_objective_done(o)
   campaign_message_box(training_2)
   p1:allow_buildings{"frisians_fortress", "frisians_tower"}
   o = add_campaign_objective(obj_defeat_enemy)
   while not p2.defeated do sleep(4273) end
   set_objective_done(o)
   
   sleep(2000)
   scroll_to_field(wl.Game().map.player_slots[2].starting_field)
   campaign_message_box(rising_water_1)
   campaign_message_box(rising_water_2)
   
   --Stormflood!
   p1.see_all = true
   scroll_to_field(firstToFlood)
   run(stormflood)
   sleep(3000)
   
   campaign_message_box(rising_water_3)
   campaign_message_box(rising_water_4)
   p1:allow_buildings{"frisians_port", "frisians_weaving_mill", "frisians_shipyard"}
   o = add_campaign_objective(obj_escape)
   
   -- wait until an expedition ship is ready
   local expReady = false
   while not expReady do
      for idx,ship in ipairs(p1:get_ships()) do
         expReady = expReady or ( ship.state:sub(1, 4) == "exp_" )
      end
      sleep(4273)
   end
   --We escaped!
   scroll_to_field(p1:get_buildings("frisians_port")[1].fields[1])
   sleep(1000)
   campaign_message_box(victory_1)
   p1:reveal_scenario("frisians01")
end

run(mission_thread)
