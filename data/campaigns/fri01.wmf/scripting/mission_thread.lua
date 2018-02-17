include "scripting/messages.lua"

shewWarningEarlyAttack = false
shewnWarningReed = false
shewnWarningClay = false
shewnWarningBricks = false

function checkWarningEarlyAttack ()
   local y = 0
   local westernmostEnemy = map.width - 1
   local easternmostOwn = 0
   while not shewWarningEarlyAttack do
      for x=0,map.width - 1 do
         local field = map:get_field (x, y)
         if field.owner == p1 and x > easternmostOwn then easternmostOwn = x end
         if field.owner == p2 and x < westernmostEnemy then westernmostEnemy = x end
         if westernmostEnemy - easternmostOwn < 21 then --if they are so close that they´d be inside a new tower´s vision range
            campaign_message_box (warning_early_attack)
            return
         end
      end
      y = y + 1
      while y >= map.height do y = y - map.height end
      sleep (50) --maximum delay = height×sleeptime = 128×50ms = 6,4s
   end
end

function warningReed ()
   local ready = 0 --show warning only if player is short on reed for more than 8 checks (= 1.5 minutes)
   while not shewnWarningReed do
      sleep (10000)
      if count ("thatch_reed") < 4 then
         if ready > 8 then
            if #p1:get_buildings ("frisians_reed_farm") < 3 then
               campaign_message_box (warning_reed)
            end
            return
         else
            ready = ready + 1
         end
      else
         ready = 0
      end
   end
end

function warningBricks ()
   local ready = 0 --show warning only if player is short on bricks for more than 5 checks (= 1 minute)
   while not shewnWarningBricks do
      sleep (10000)
      if count ("brick") < 4 then
         if ready > 5 then
            if #p1:get_buildings ("frisians_brick_burners_house") < 3 then
               campaign_message_box (warning_bricks)
            end
            return
         else
            ready = ready + 1
         end
      else
         ready = 0
      end
   end
end

function warningClay ()
   local ready = 0 --show warning only if player is short on reed for more than 11 checks (= 2 minutes)
   while not shewnWarningClay do
      sleep (10000)
      if count ("clay") < 4 then
         if ready > 11 then
            if #p1:get_buildings ("frisians_clay_pit") < 4 then
               campaign_message_box (warning_clay)
            end
            return
         else
            ready = ready + 1
         end
      else
         ready = 0
      end
   end
end

function count (ware)
   local whs = array_combine (
      p1:get_buildings ("frisians_headquarters"),
      p1:get_buildings ("frisians_port"),
      p1:get_buildings ("frisians_warehouse"))
   local inWH = 0
   for idx,wh in ipairs (whs) do
      inWH = inWH + wh:get_wares (ware)
   end
   return inWH
end

function stormflood ()
   local x
   local y
   local yD
   local up
   local check
   local nextF = firstToFlood
   while not (nextF == nil) do
      nextF.terr = "winter_water"
      nextF.terd = "winter_water"
      --turn the settlement into Rungholt; even swimming things (like ships) are destroyed by the storm
      for idx,field in ipairs (nextF:region (1)) do
         if field.immovable then
            local fs = field.immovable.fields
            field.immovable:remove ()
            for idy,f in ipairs (fs) do map:place_immovable ("ashes", f, "tribes") end
         end
         for idb,bob in ipairs (field.bobs) do bob:remove () end
      end
      x = nextF.x
      nextF = nil
      while (nextF == nil) and (x < map.width) do
         yD = 0
         up = true
         while (nextF == nil) and (yD < (map.height / 2)) do
            y = firstToFlood.y
            if up then y = y + yD else y = y - yD end
            if y < 0 then y = y + map.height end
            if not (y < map.height) then y = y - map.height end
            check = map:get_field (x, y)
            if (not ((check.terr == "winter_water") and (check.terd == "winter_water"))) and
               (check.height < 4) then nextF = check end
            if up then yD = yD + 1 end
            up = not up
         end
         x = x + 1
      end
      sleep (413)
   end
end

function mission_thread ()

   --Introduction
   sleep (1000)
   campaign_message_box (intro_1)
   include "map:scripting/starting_conditions.lua"
   scroll_to_field (map.player_slots [1].starting_field)
   sleep (5000)
   campaign_message_box (intro_2)
   campaign_message_box (intro_3)
   --build wood economy and reed yard
   p1:allow_buildings {"frisians_woodcutters_house", "frisians_foresters_house", "frisians_reed_farm"}
   local o = add_campaign_objective (obj_build_wood_economy)
   while not check_for_buildings (p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 2,
      frisians_reed_farm = 1}) do sleep (4273) end
   set_objective_done (o)

   campaign_message_box (intro_4)
   --build brick economy
   p1:allow_buildings {"frisians_brick_burners_house", "frisians_well", "frisians_clay_pit", "frisians_coalmine",
      "frisians_rockmine"}
   o = add_campaign_objective (obj_build_brick_economy)
   run (warningReed)
   while not check_for_buildings (p1, {frisians_brick_burners_house = 1, frisians_well = 1, frisians_clay_pit = 1,
      frisians_coalmine = 1, frisians_rockmine = 1}) do sleep (4273) end
   set_objective_done (o)
   campaign_message_box (intro_5)
   run (warningClay)

   --wait until no rations left
   sleep (60000)
   local somethingLeft = true
   while somethingLeft do
      local inWH = count ("ration")
      local whs = array_combine(
         p1:get_buildings ("frisians_coalmine"),
         p1:get_buildings ("frisians_rockmine")
      )
      local mW = false -- has at least 1 mine no rations left?
      for idx,wh in ipairs (whs) do
         mW = mW or (wh:get_inputs ("ration") == 0)
      end
      somethingLeft = (inWH > 0) or not mW
      sleep (4273)
   end

   --great, you forgot to provide rations…
   campaign_message_box (food_1)
   run (warningBricks)
   p1:allow_buildings {"frisians_fishers_house", "frisians_smokery", "frisians_hunters_house", "frisians_tavern",
      "frisians_berry_farm", "frisians_collectors_house", "frisians_outpost"}
   o = add_campaign_objective (obj_build_food_economy)
   while not check_for_buildings (p1, {frisians_fishers_house = 1, frisians_smokery = 1, frisians_tavern = 1,
      frisians_berry_farm = 1, frisians_collectors_house = 1}) do sleep (4273) end
   set_objective_done (o)

   --we want better food
   campaign_message_box (food_2)
   p1:allow_buildings {"frisians_beekeepers_house", "frisians_farm", "frisians_bakery", "frisians_brewery",
      "frisians_mead_brewery", "frisians_honey_bread_bakery"}
   o = add_campaign_objective (obj_build_food_economy_2)
   while not check_for_buildings (p1, {frisians_tavern = 2}) do sleep (4273) end
   p1:allow_buildings {"frisians_drinking_hall"}
   while not check_for_buildings (p1, {frisians_farm = 2, frisians_bakery = 1, frisians_brewery = 1,
      frisians_beekeepers_house = 1}) do sleep (4273) end
   set_objective_done (o)

   --we can start some real mining
   campaign_message_box (mining_1)
   p1:allow_buildings {"frisians_ironmine", "frisians_goldmine", "frisians_coalmine_deep", "frisians_rockmine_deep",
       "frisians_ironmine_deep", "frisians_goldmine_deep", "frisians_furnace", "frisians_blacksmithy",
       "frisians_armor_smithy_small", "frisians_charcoal_kiln"}
   p2:allow_buildings {"frisians_outpost"}
   o = add_campaign_objective (obj_build_mining)
   while not check_for_buildings (p1, {frisians_ironmine = 1, frisians_furnace = 1, frisians_blacksmithy = 1,
      frisians_armor_smithy_small = 1}) do sleep (4273) end
   set_objective_done (o)

   --start recruiting
   campaign_message_box (recruit_1)
   campaign_message_box (recruit_2)
   o = add_campaign_objective (obj_recruit_soldiers)
   p1:allow_buildings {"frisians_barracks", "frisians_reindeer_farm", "frisians_seamstress", "frisians_fortress"}
   while not check_for_buildings (p1, {frisians_barracks = 1, frisians_seamstress = 1,
      frisians_reindeer_farm = 1}) do sleep (4273) end
   set_objective_done (o)

   --show the "expand" objective only if we haven´t expanded that far yet
   local noskip = expansionMark.owner == nil
   if noskip then
      campaign_message_box (expand_1)
      o = add_campaign_objective (obj_expand)
   end

   --wait until we (or the enemy) have conquered the left half of the island
   while expansionMark.owner == nil do sleep (4273) end
   if noskip then set_objective_done (o) end

   --a friendly chat between neighbours
   p1:reveal_fields (map.player_slots [2].starting_field:region (6))
   scroll_to_field (map.player_slots [2].starting_field)
   sleep (1000)
   campaign_message_box (enemies_1)
   campaign_message_box (enemies_2)
   campaign_message_box (enemies_3)
   scroll_to_field (expansionMark)
   p1:hide_fields (map.player_slots [2].starting_field:region (6))
   sleep (1000)

   --start training
   campaign_message_box (training_1)
   campaign_message_box (training_2)
   campaign_message_box (training_3)
   p1:allow_buildings {"frisians_training_camp", "frisians_training_arena", "frisians_seamstress_master",
      "frisians_armor_smithy_large", "frisians_wooden_tower", "frisians_wooden_tower_high", "frisians_scouts_house"}
   p2:allow_buildings {"frisians_fortress", "frisians_reindeer_farm"}
   o = add_campaign_objective (obj_train_soldiers)

   --wait until at least 1 soldier has level 10
   local skipToFlood = false
   local hasL10 = false
   run (checkWarningEarlyAttack)
   while not (hasL10 or skipToFlood) do
      local bld = array_combine (
         p1:get_buildings ("frisians_headquarters"),
         p1:get_buildings ("frisians_warehouse"),
         p1:get_buildings ("frisians_port"),
         p1:get_buildings ("frisians_sentinel"),
         p1:get_buildings ("frisians_fortress"),
         p1:get_buildings ("frisians_tower"),
         p1:get_buildings ("frisians_wooden_tower"),
         p1:get_buildings ("frisians_wooden_tower_high"),
         p1:get_buildings ("frisians_outpost")
      )
      for idx,site in ipairs (bld) do
         hasL10 = hasL10 or (site:get_soldiers {2,6,2,0} > 0)
      end
      skipToFlood = skipToFlood or p2.defeated
      sleep (4273)
   end
   shewWarningEarlyAttack = true --We are strong enough now – no need for the warning if it didn´t appear yet
   shewnWarningBricks = true

   --Attack!
   set_objective_done (o)
   p1:allow_buildings {"frisians_tower"}
   p2:allow_buildings {"frisians_barracks", "frisians_seamstress", "frisians_armor_smithy_small"}
   if not skipToFlood then
      campaign_message_box (training_4)
      campaign_message_box (training_5)
      campaign_message_box (training_6)
      o = add_campaign_objective (obj_defeat_enemy)
      while not p2.defeated do sleep (4273) end
      set_objective_done (o)
   end
   shewnWarningReed = true
   shewnWarningClay = true

   sleep (2000)
   scroll_to_field (map.player_slots [2].starting_field)
   campaign_message_box (rising_water_1)
   campaign_message_box (rising_water_2)

   --Stormflood!
   for x=4,map.width - 5 do
      for y=4,map.height - 5 do --leave some small margin, show everything else
         p1:reveal_fields {map:get_field (x, y)}
      end
   end
   scroll_to_field (firstToFlood)
   run (stormflood)
   sleep (6000)

   campaign_message_box (rising_water_3)
   campaign_message_box (rising_water_4)
   p1:allow_buildings {"frisians_port", "frisians_weaving_mill", "frisians_shipyard"}
   o = add_campaign_objective (obj_escape)

   -- wait until an expedition ship is ready
   local expReady = false
   while not expReady do
      for idx,ship in ipairs (p1:get_ships ()) do
         expReady = expReady or (ship.state:sub (1, 4) == "exp_")
      end
      sleep (1149)
   end
   --We escaped!
   scroll_to_field (p1:get_buildings ("frisians_port") [1].fields [1])
   sleep (1000)
   campaign_message_box (victory_1)
   p1:reveal_scenario ("frisians01")
   --END OF MISSION 1
end

run (mission_thread)
