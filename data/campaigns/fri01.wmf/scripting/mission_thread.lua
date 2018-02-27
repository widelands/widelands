include "scripting/messages.lua"

show_warning_early_attack = true
show_warning_reed = true
show_warning_clay = true
show_warning_bricks = true

function hint_warehouse_east()
   local fields = warehouse_mark:region(5)
   for i,f in ipairs(fields) do
      sleep(123)
      if f.owner == p1 then
         campaign_message_box(warehouse_on_expand)
         return
      end
   end
end

function check_warning_early_attack()
   local y = 0
   local westernmost_enemy = map.width - 1
   local easternmost_own = 0
   while show_warning_early_attack do
      for x=0,map.width - 1 do
         local field = map:get_field(x, y)
         if field.owner == p1 and x > easternmost_own then easternmost_own = x end
         if field.owner == p2 and x < westernmost_enemy then westernmost_enemy = x end
         -- If they are so close that they'd be inside a new tower's vision range
         if westernmost_enemy - easternmost_own < 21 then
            campaign_message_box(warning_early_attack)
            return
         end
      end
      y = y + 1
      while y >= map.height do y = y - map.height end
      sleep(50) -- maximum delay = height×sleeptime = 128×50ms = 6,4s
   end
end

function warning_reed()
   -- Show warning only if player is short on reed for more than 8 checks (= 1.5 minutes)
   local ready = 0
   while show_warning_reed do
      sleep(10000)
      if count("thatch_reed") < 4 then
         if ready > 8 then
            if #p1:get_buildings("frisians_reed_farm") < 3 then
               campaign_message_box(warning_reed)
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

function warning_bricks()
   -- Show warning only if player is short on bricks for more than 5 checks (= 1 minute)
   local ready = 0
   while show_warning_bricks do
      sleep(10000)
      if count("brick") < 4 then
         if ready > 5 then
            if #p1:get_buildings("frisians_brick_burners_house") < 3 then
               campaign_message_box(warning_bricks)
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

function warning_clay()
   -- Show warning only if player is short on reed for more than 11 checks (= 2 minutes)
   local ready = 0
   while show_warning_clay do
      sleep(10000)
      if count("clay") < 4 then
         if ready > 11 then
            if #p1:get_buildings("frisians_clay_pit") < 4 then
               campaign_message_box(warning_clay)
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

function count(ware)
   local whs = array_combine(
      p1:get_buildings("frisians_headquarters"),
      p1:get_buildings("frisians_port"),
      p1:get_buildings("frisians_warehouse"))
   local in_warehouses = 0
   for idx,wh in ipairs(whs) do
      in_warehouses = in_warehouses + wh:get_wares(ware)
   end
   return in_warehouses
end

function stormflood()
   local x
   local y
   local delta_y
   local up
   local check
   local next_field = first_to_flood
   while next_field ~= nil do
      next_field.terr = "winter_water"
      next_field.terd = "winter_water"
      -- Turn the settlement into Rungholt; even swimming things (like ships)
      -- are destroyed by the storm
      for idx,field in ipairs(next_field:region(1)) do
         if field.immovable then
            local fs = nil -- NOCOM what does fs mean?
            if is_building(field.immovable) then fs = field.immovable.fields end
            field.immovable:remove()
            if fs ~= nil then
               for idy,f in ipairs(fs) do map:place_immovable("ashes", f, "tribes") end
            end
         end
         for idb,bob in ipairs(field.bobs) do bob:remove() end
      end
      x = next_field.x
      next_field = nil
      while (next_field == nil) and (x < map.width) do
         delta_y = 0
         up = true
         while (next_field == nil) and (delta_y < (map.height / 2)) do
            y = first_to_flood.y
            if up then
               y = y + delta_y
            else
               y = y - delta_y
            end
            if y < 0 then
               y = y + map.height
            end
            if y >= map.height then
               y = y - map.height
            end
            check = map:get_field(x, y)
            if (check.terr ~= "winter_water" or check.terd ~= "winter_water") and
               (check.height < 4) then
               next_field = check
            end
            if up then
               delta_y = delta_y + 1
            end
            up = not up
         end
         x = x + 1
      end
      sleep(413)
   end
end

function mission_thread()

   --Introduction
   sleep(1000)
   campaign_message_box(intro_1)
   include "map:scripting/starting_conditions.lua"
   scroll_to_field(map.player_slots[1].starting_field)
   sleep(5000)
   campaign_message_box(intro_2)
   campaign_message_box(intro_3)

   -- Build wood economy and reed yard
   p1:allow_buildings {"frisians_woodcutters_house", "frisians_foresters_house", "frisians_reed_farm"}
   local o = add_campaign_objective(obj_build_wood_economy)
   while not check_for_buildings(p1, {
      frisians_woodcutters_house = 1,
      frisians_foresters_house = 2,
      frisians_reed_farm = 1}) do sleep(4273) end
   set_objective_done(o)

   campaign_message_box(intro_4)

   -- Build brick economy
   p1:allow_buildings {
      "frisians_brick_burners_house",
      "frisians_well",
      "frisians_clay_pit",
      "frisians_coalmine",
      "frisians_rockmine"}
   o = add_campaign_objective(obj_build_brick_economy)
   run(warning_reed)
   while not check_for_buildings(p1, {
      frisians_brick_burners_house = 1,
      frisians_well = 1,
      frisians_clay_pit = 1,
      frisians_coalmine = 1,
      frisians_rockmine = 1}) do sleep(4273) end
   set_objective_done(o)
   campaign_message_box(intro_5)
   run(warning_clay)

   run(hint_warehouse_east)

   -- Wait until no rations left
   local something_left = true
   while something_left do
      sleep(4273)
      local in_warehouses = count("ration")
      local whs = array_combine(
         p1:get_buildings("frisians_coalmine"),
         p1:get_buildings("frisians_rockmine")
      )
      local m_w = false -- Has at least 1 mine no rations left?
      for idx,wh in ipairs(whs) do
         m_w = m_w or (wh:get_inputs("ration") == 0)
      end
      something_left = (in_warehouses > 0) or not m_w
   end

   -- Great, you forgot to provide rations...
   campaign_message_box(food_1)
   run(warning_bricks)
   p1:allow_buildings {
      "frisians_fishers_house",
      "frisians_smokery",
      "frisians_hunters_house",
      "frisians_tavern",
      "frisians_berry_farm",
      "frisians_collectors_house",
      "frisians_outpost"}
   o = add_campaign_objective(obj_build_food_economy)
   while not check_for_buildings(p1, {
      frisians_fishers_house = 1,
      frisians_smokery = 1,
      frisians_tavern = 1,
      frisians_berry_farm = 1,
      frisians_collectors_house = 1}) do sleep(4273) end
   set_objective_done(o)

   -- We want better food
   campaign_message_box(food_2)
   p1:allow_buildings {
      "frisians_beekeepers_house",
      "frisians_farm",
      "frisians_bakery",
      "frisians_brewery",
      "frisians_mead_brewery",
      "frisians_honey_bread_bakery",
      "frisians_ironmine",
      "frisians_furnace",
      "frisians_blacksmithy"}
   o = add_campaign_objective(obj_build_food_economy_2)
   while not check_for_buildings(p1, {frisians_tavern = 2}) do sleep(4273) end
   p1:allow_buildings {"frisians_drinking_hall"}
   while not check_for_buildings(p1, {
      frisians_farm = 2,
      frisians_bakery = 1,
      frisians_brewery = 1,
      frisians_beekeepers_house = 1}) do sleep(4273) end
   set_objective_done(o)

   -- We can start some real mining
   campaign_message_box(mining_1)
   p1:allow_buildings {
      "frisians_goldmine",
      "frisians_coalmine_deep",
      "frisians_rockmine_deep",
      "frisians_ironmine_deep",
      "frisians_goldmine_deep",
      "frisians_armor_smithy_small",
      "frisians_charcoal_kiln"}
   p2:allow_buildings {"frisians_outpost"}
   o = add_campaign_objective(obj_build_mining)
   while not check_for_buildings(p1, {
      frisians_ironmine = 1,
      frisians_furnace = 1,
      frisians_blacksmithy = 1,
      frisians_armor_smithy_small = 1}) do sleep(4273) end
   set_objective_done(o)

   -- Start recruiting
   campaign_message_box(recruit_1)
   campaign_message_box(recruit_2)
   o = add_campaign_objective(obj_recruit_soldiers)
   p1:allow_buildings {
      "frisians_barracks",
      "frisians_reindeer_farm",
      "frisians_sewing_room",
      "frisians_fortress"}
   while not check_for_buildings(p1, {
      frisians_barracks = 1,
      frisians_sewing_room = 1,
      frisians_reindeer_farm = 1}) do sleep(4273) end
   set_objective_done(o)

   -- Show the "expand" objective only if we haven't expanded that far yet
   if expansion_mark.owner == nil then
      campaign_message_box(expand_1)
      o = add_campaign_objective(obj_expand)
      -- Wait until we (or the enemy) have conquered the left half of the island
      while expansion_mark.owner == nil do sleep(4273) end
      set_objective_done(o)
   end



   -- A friendly chat between neighbours
   p1:reveal_fields(map.player_slots[2].starting_field:region(6))
   scroll_to_field(map.player_slots[2].starting_field)
   sleep(1000)
   campaign_message_box(enemies_1)
   campaign_message_box(enemies_2)
   campaign_message_box(enemies_3)
   scroll_to_field(expansion_mark)
   p1:hide_fields(map.player_slots[2].starting_field:region(6))
   sleep(1000)

   -- Start training
   campaign_message_box(training_1)
   campaign_message_box(training_2)
   campaign_message_box(training_3)
   p1:allow_buildings {
      "frisians_training_camp",
      "frisians_training_arena",
      "frisians_tailors_shop",
      "frisians_armor_smithy_large",
      "frisians_wooden_tower",
      "frisians_wooden_tower_high",
      "frisians_scouts_house"}
   p2:allow_buildings {"frisians_fortress", "frisians_reindeer_farm"}
   o = add_campaign_objective(obj_train_soldiers)

   -- Wait until at least 1 soldier has level 10
   local skip_to_flood = false
   local has_hero = false
   run(check_warning_early_attack)
   while not (has_hero or skip_to_flood) do
      local bld = array_combine(
         p1:get_buildings("frisians_headquarters"),
         p1:get_buildings("frisians_warehouse"),
         p1:get_buildings("frisians_port"),
         p1:get_buildings("frisians_sentinel"),
         p1:get_buildings("frisians_fortress"),
         p1:get_buildings("frisians_tower"),
         p1:get_buildings("frisians_wooden_tower"),
         p1:get_buildings("frisians_wooden_tower_high"),
         p1:get_buildings("frisians_outpost")
      )
      for idx,site in ipairs(bld) do
         has_hero = has_hero or (site:get_soldiers {2,6,2,0} > 0)
      end
      skip_to_flood = skip_to_flood or p2.defeated
      sleep(4273)
   end
   -- We are strong enough now – no need for the warning if it didn't appear yet
   show_warning_early_attack = false
   show_warning_bricks = false

   -- Attack!
   set_objective_done(o)
   p1:allow_buildings {"frisians_tower"}
   p2:allow_buildings {
      "frisians_barracks",
      "frisians_sewing_room",
      "frisians_armor_smithy_small"}
   if not skip_to_flood then
      campaign_message_box(training_4)
      campaign_message_box(training_5)
      campaign_message_box(training_6)
      o = add_campaign_objective(obj_defeat_enemy)
      while not p2.defeated do sleep(4273) end
      set_objective_done(o)
   end
   show_warning_reed = false
   show_warning_clay = false

   sleep(2000)
   scroll_to_field(map.player_slots[2].starting_field)
   campaign_message_box(rising_water_1)
   campaign_message_box(rising_water_2)

   -- Stormflood!
   for x=3,map.width - 4 do
      for y=3,map.height - 4 do -- Leave some small margin, show everything else
         p1:reveal_fields {map:get_field(x, y)}
      end
   end
   scroll_to_field(first_to_flood)
   run(stormflood)
   sleep(6000)

   campaign_message_box(rising_water_3)
   campaign_message_box(rising_water_4)
    -- Make the port space immediately accessible if it is blocked with trees etc
    -- (but not buildings)
   for i,f in ipairs(port_space:region(3)) do
      if f.immovable and f.immovable.descr.type_name == "immovable" then f.immovable:remove() end
   end
   scroll_to_field(port_space)
   p1:allow_buildings {"frisians_port", "frisians_weaving_mill", "frisians_shipyard"}
   o = add_campaign_objective(obj_escape)

   -- Wait until an expedition ship is ready
   local expedition_ready = false
   while not expedition_ready do
      for idx,ship in ipairs(p1:get_ships()) do
         expedition_ready = expedition_ready or (ship.state:sub(1, 4) == "exp_")
      end
      sleep(1149)
   end

   -- We escaped!
   scroll_to_field(p1:get_buildings("frisians_port")[1].fields[1])
   sleep(1000)
   campaign_message_box(victory_1)
   p1:reveal_scenario("frisians01")
   -- END OF MISSION 1
end

run(mission_thread)
