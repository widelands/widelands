include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"
include "scripting/field_animations.lua"

-- Some objectives need to be waited for in separate threads
local obj_find_monastery_done = false
local julia_conquered = false
local enemy = false
local mv = wl.ui.MapView()

--  dismantle the unproductive small buildings
function dismantle()
   local o = add_campaign_objective(obj_dismantle_buildings)
   local buildmessage = false
   sleep(5000)
   while count_buildings(p1, {"empire_fishers_house", "empire_quarry", "empire_lumberjacks_house2", "empire_well2"}) > 0 do
      if mv.windows.field_action and mv.windows.field_action.tabs.small and not buildmessage then
         campaign_message_box(amalea_19)
         buildmessage = true
      end
      sleep(500)
   end
   sleep(2000)
   p1:allow_buildings("all")
   p1:forbid_buildings{"empire_farm", "empire_mill", "empire_brewery", "empire_trainingcamp", "empire_colosseum"}
   o.done = true
   campaign_message_box(amalea_3)
   run(clear_roads)
   run(quarries_lumberjacks)
end

-- we need to find the plans how to build a farm
function farm_plans()
   local f = map:get_field(47, 10)
   local farmclick = false
   local count = 0
   while not farmclick do
      if mv.windows.building_window and not mv.windows.building_window.buttons.dismantle and not mv.windows.building_window.tabs.wares and mv.windows.building_window.tabs.workers then
         farmclick = true
      end
      count = count + 1
      if count == 1201 then
         campaign_message_box(amalea_18)
      end
      sleep(500)
   end

   campaign_message_box(amalea_2)
   local o = add_campaign_objective(obj_find_farm_plans)
   while not (f.owner == p1) do
      sleep(4000)
   end
   if f.immovable then
      local prior_center = scroll_to_field(f)
      campaign_message_box(amalea_4)
      sleep(2000)
      f.immovable:remove()
      sleep(2000)
      scroll_to_map_pixel(prior_center)
   else
      print("Failed to remove artifact at (" .. f.x .. ", " .. f.y .. ")")
   end

   o.done = true
   p1:allow_buildings{"empire_farm"}
   run (wheat_chain)
end

-- the chaotic road network has to be cleared
function clear_roads()
   local o = add_campaign_objective(obj_clear_roads)
   local cleared = false
   local count_deadend = 0
   local count_star = 0
   local timer = 0

   while not cleared do
      count_deadend = 0
      count_star = 0
      cleared = true
      sleep (5000)
      for x=5, 40 do
         for y=0, 56 do
            local field = map:get_field(x,y)
            if (field.immovable and field.immovable.descr.type_name == "flag" and field.immovable.building == nil) then
               local numroads = 0
               for _ in pairs(field.immovable.roads) do numroads = numroads + 1 end
               if numroads < 2 then
                  count_deadend = count_deadend + 1
               elseif numroads > 4 then
                  count_star = count_star + 1
               end
            end
         end
      end
      if count_deadend > 3 or count_star > 1 then
         cleared = false
      end
      timer = timer + 1
      if timer == 100 then
         campaign_message_box(amalea_20)
      end
   end
   o.done = true
   campaign_message_box(amalea_6)
end

-- the foresters have to be replaced too
function no_trees()
   local trees = 100
   while trees > 8 do
      trees = 0
      sleep (5000)
      for x=13, 25 do
         for y= 2, 25 do
            local field = map:get_field(x,y)
            if (field.immovable and field.immovable.descr.terrain_affinity) then
               trees = trees + 1
            end
         end
      end
   end
   if #p1:get_buildings("empire_foresters_house2") > 0 then
      local o = add_campaign_objective(obj_replace_foresters)
      campaign_message_box(amalea_7)
      while #p1:get_buildings("empire_foresters_house") < 2 do sleep(3249) end
      o.done = true
      campaign_message_box(amalea_8)
   end
end

-- after having some logs and planks we need to ensure a constant supply of building materials
function quarries_lumberjacks()
   local o = add_campaign_objective(obj_build_quarries_and_lumberjacks)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 3, empire_quarry = 2}) do sleep(3000) end
   o.done = true
   campaign_message_box(amalea_5)
   run(produce_food)
   run(no_trees)
end

-- now we can start to produce food for our miners
function produce_food()
   local o = add_campaign_objective(obj_produce_fish)
   while p1:get_produced_wares_count("ration") < 14 do sleep(3000) end
   o.done = true
   run(steel)
   run(charcoal)
end

-- after having started the metal production we need tools and later on we need soldiers
function steel()
   campaign_message_box(amalea_13)
   local o = add_campaign_objective(obj_produce_tools)

   while ((
         p1:get_produced_wares_count("basket") +
         p1:get_produced_wares_count("bread_paddle") +
         p1:get_produced_wares_count("felling_ax") +
         p1:get_produced_wares_count("fire_tongs") +
         p1:get_produced_wares_count("fishing_rod") +
         p1:get_produced_wares_count("hammer") +
         p1:get_produced_wares_count("hunting_spear") +
         p1:get_produced_wares_count("kitchen_tools") +
         p1:get_produced_wares_count("pick") +
         p1:get_produced_wares_count("saw") +
         p1:get_produced_wares_count("scythe") +
         p1:get_produced_wares_count("shovel")
      ) < 10) do
   sleep(2500)
   end
   campaign_message_box(diary_page_2)
   o.done = true
   sleep(10000)
   run(check_enemy)

   -- enough tools produced now start to build weapons
   local o1 = add_campaign_objective(obj_recruit_soldiers)
   campaign_message_box(saledus_5)
   local number_soldiers = 0
   local bld = array_combine(
      p1:get_buildings("empire_headquarters"),
      p1:get_buildings("empire_warehouse"),
      p1:get_buildings("empire_trainingcamp1"),
      p1:get_buildings("empire_arena"),
      p1:get_buildings("empire_sentry"),
      p1:get_buildings("empire_tower"),
      p1:get_buildings("empire_fortress"),
      p1:get_buildings("empire_outpost"),
      p1:get_buildings("empire_barrier"),
      p1:get_buildings("empire_blockhouse"),
      p1:get_buildings("empire_castle")
   )
   for idx,site in ipairs(bld) do
      for descr,count in pairs(site:get_soldiers("all")) do
         number_soldiers = number_soldiers + count
      end
   end

   local enough_soldiers = false
   while not enough_soldiers do
      bld = array_combine(
         p1:get_buildings("empire_headquarters"),
         p1:get_buildings("empire_warehouse"),
         p1:get_buildings("empire_trainingcamp1"),
         p1:get_buildings("empire_arena"),
         p1:get_buildings("empire_sentry"),
         p1:get_buildings("empire_tower"),
         p1:get_buildings("empire_fortress"),
         p1:get_buildings("empire_outpost"),
         p1:get_buildings("empire_barrier"),
         p1:get_buildings("empire_blockhouse"),
         p1:get_buildings("empire_castle")
      )
      local amount = 0
      for idx,site in ipairs(bld) do
         for descr,count in pairs(site:get_soldiers("all")) do
            amount = amount + count
         end
      end
      if amount > number_soldiers + 9 then
         enough_soldiers = true
      end
      sleep(4273)
   end
   o1.done = true
   campaign_message_box(saledus_6)
   run(training)
end

-- charcoal might be needed to support the metal production
function charcoal()
   while p1:get_wares("coal") < 15 do sleep(2342) end
   while (p1:get_wares("coal")/p1:get_wares("iron_ore")) > 0.1 do sleep(2342) end
   local o = add_campaign_objective(obj_charcoal)
   campaign_message_box(amalea_14)
   while #p1:get_buildings("empire_charcoal_kiln") < 2 do sleep(3249) end
   o.done = true
   campaign_message_box(amalea_15)
end

-- soldiers need to be trained until we have 3 heroes
function training()
   local o = add_campaign_objective(obj_training)
   local strength = 0

   local bld = array_combine(
      p1:get_buildings("empire_headquarters"),
      p1:get_buildings("empire_warehouse"),
      p1:get_buildings("empire_trainingcamp1"),
      p1:get_buildings("empire_arena"),
      p1:get_buildings("empire_sentry"),
      p1:get_buildings("empire_tower"),
      p1:get_buildings("empire_fortress"),
      p1:get_buildings("empire_outpost"),
      p1:get_buildings("empire_barrier"),
      p1:get_buildings("empire_blockhouse"),
      p1:get_buildings("empire_castle")
   )
   for idx,site in ipairs(bld) do
      for descr,count in pairs(site:get_soldiers("all")) do
         strength = strength + descr[1]*count + descr[2]*count
      end
   end

   local enough_strength = false
   while not enough_strength do
      bld = array_combine(
         p1:get_buildings("empire_headquarters"),
         p1:get_buildings("empire_warehouse"),
         p1:get_buildings("empire_trainingcamp1"),
         p1:get_buildings("empire_arena"),
         p1:get_buildings("empire_sentry"),
         p1:get_buildings("empire_tower"),
         p1:get_buildings("empire_fortress"),
         p1:get_buildings("empire_outpost"),
         p1:get_buildings("empire_barrier"),
         p1:get_buildings("empire_blockhouse"),
         p1:get_buildings("empire_castle")
      )
      local amount = 0
      for idx,site in ipairs(bld) do
         for descr,count in pairs(site:get_soldiers("all")) do
            amount = amount + descr[1]*count + descr[2]*count
         end
      end
      if amount > strength + 10 then
         enough_strength = true
      end
      sleep(4273)
   end
   o.done = true

   -- after some training we have enough knowledge to build better training buildings
   p1:allow_buildings{"empire_trainingcamp", "empire_colosseum"}
   campaign_message_box(saledus_7)
   local o2 = add_campaign_objective(obj_upgrade)
   sleep(5000)

   while not (check_for_buildings(p1, { empire_trainingcamp = 1, empire_colosseum = 1})) do sleep(3000) end
   o2.done = true
   campaign_message_box(diary_page_3)
   sleep(5000)
   while not (obj_find_monastery_done) do sleep(2000) end
   campaign_message_box(saledus_8)
   local o1 = add_campaign_objective(obj_heroes)
   local heroes = false

   while not heroes do
      bld = array_combine(
         p1:get_buildings("empire_headquarters"),
         p1:get_buildings("empire_warehouse"),
         p1:get_buildings("empire_trainingcamp1"),
         p1:get_buildings("empire_trainingcamp"),
         p1:get_buildings("empire_arena"),
         p1:get_buildings("empire_colosseum"),
         p1:get_buildings("empire_sentry"),
         p1:get_buildings("empire_tower"),
         p1:get_buildings("empire_fortress"),
         p1:get_buildings("empire_outpost"),
         p1:get_buildings("empire_barrier"),
         p1:get_buildings("empire_blockhouse"),
         p1:get_buildings("empire_castle")
      )
      local amount = 0
      for idx,site in ipairs(bld) do
         amount = amount + (site:get_soldiers{4,4,0,2})
      end
      if amount > 2 then
         heroes = true
      end
      sleep(4273)
   end

   o1.done = true
   if enemy == false then
      enemy = true
      campaign_message_box(saledus_9)
      run(conquer)
   end
end

-- check if the enemy has been seen and where
function check_enemy()
   local en_see = {}
   while not enemy do
      en_see = enemy_seen()
      if en_see then
         local prior_center = scroll_to_field(en_see)
         sleep(2000)
         campaign_message_box(saledus_11)
         enemy = true
         run(conquer)
         scroll_to_map_pixel(prior_center)
      end
      sleep(8000)
   end
end

-- lets finish the babarians off
function conquer()
   local o = add_campaign_objective(obj_conquer_all)
   while not p2.defeated do sleep(2342) end
   o.done = true

   -- Babarians defeated.
   campaign_message_box(saledus_10)

   -- Sleep a while to have some time between the last objective done message and final victory
   sleep(25000)
   campaign_message_box(diary_page_4)

   p1:reveal_campaign("campsect2")
   p1:reveal_scenario("empiretut04")
end

-- another production chain that is ineffective and needs to be corrected
function wheat_chain()
   while not (p1:get_produced_wares_count('beer') > 4  and p1:get_produced_wares_count('flour') > 4) do sleep(2434) end
   local o = add_campaign_objective(obj_find_monastery)
   campaign_message_box(amalea_9)
   while not (
      p1:sees_field(map:get_field(16,184)) or
      p1:sees_field(map:get_field(16,185)) or
      p1:sees_field(map:get_field(17,186)) or
      p1:sees_field(map:get_field(17,187)) or
      p1:sees_field(map:get_field(18,188)) or
      p1:sees_field(map:get_field(18,189)) or
      p1:sees_field(map:get_field(19,190)) or
      p1:sees_field(map:get_field(20,190)) or
      p1:sees_field(map:get_field(21,190)) or
      p1:sees_field(map:get_field(22,190)) or
      p1:sees_field(map:get_field(23,190)) or
      p1:sees_field(map:get_field(24,190))) do
      sleep(2500)
   end

   local field_well = map:get_field(17, 182)
   place_building_in_region(p3, "empire_well", {field_well})

   local field_brewery = map:get_field(19, 183)
   place_building_in_region(p3, "empire_brewery", {field_brewery})

   local fiel_mill = map:get_field(18, 184)
   place_building_in_region(p3, "empire_mill", {fiel_mill})

   local field_warehouse = map:get_field(21, 186)
   place_building_in_region(p3, "empire_warehouse", {field_warehouse}, {workers = {empire_carrier = 0, empire_recruit = 0}})

   local field_sentry = map:get_field(19, 185)
   place_building_in_region(p3, "empire_sentry", {field_sentry}, {soldiers = {[{0,0,0,0}] = 1}})

   o.done = true
   sleep(4000)

   -- Julia the priestess of Vesta appears
   local julia = map:get_field(19, 185)
   local prior_center = scroll_to_field(julia)
   sleep(200)
   reveal_concentric(p1, julia, 7, false, 150)
   campaign_message_box(julia_0)
   campaign_message_box(amalea_10)
   campaign_message_box(saledus_1)
   local o1 = add_campaign_objective(obj_deal_with_julia)
   scroll_to_map_pixel(prior_center)

   local hq = p1:get_buildings("empire_headquarters")
   local wh = p3:get_buildings("empire_warehouse")
   while not ((hq[1]:get_wares("wheat") > 34 and hq[1]:get_wares("wine") > 14) or p3.defeated) do sleep(4000) end
   if p3.defeated then
      o1.done = true
      julia_conquered = true
      p1:allow_buildings{"empire_mill", "empire_brewery"}
      campaign_message_box(saledus_2)
      campaign_message_box(julia_2)
      campaign_message_box(amalea_11)
      campaign_message_box(saledus_4)
   else
      o1.done = true
      wh[1]:set_workers("empire_carrier", 0)
      wh[1]:set_workers("empire_recruit", 0)
      local wheat = hq[1]:get_wares("wheat") - 35
      local wine = hq[1]:get_wares("wine") - 15
      hq[1]:set_wares("wheat", wheat)
      hq[1]:set_wares("wine", wine)
      p1:allow_buildings{"empire_mill", "empire_brewery"}
      campaign_message_box(julia_1)

      -- replace Julias buildings with similar ones of the player
      field_well.immovable:remove()
      field_brewery.immovable:remove()
      fiel_mill.immovable:remove()
      field_warehouse.immovable:remove()
      field_sentry.immovable:remove()
      place_building_in_region(p1, "empire_well", {field_well})
      place_building_in_region(p1, "empire_brewery", {field_brewery})
      place_building_in_region(p1, "empire_mill", {fiel_mill})
      place_building_in_region(p1, "empire_warehouse", {field_warehouse}, {wares = {water = 30, flour = 30, beer = 40,}})
      place_building_in_region(p1, "empire_sentry", {field_sentry})

      campaign_message_box(amalea_12)
      campaign_message_box(saledus_3)
   end
   obj_find_monastery_done = true
   run(karma)
end

-- our actions have an effect positively or negatively
function karma()
   if julia_conquered then
      for count = 0, 10 do
         sleep(1200000)
         bld = {
            "empire_stonemasons_house",
            "empire_sawmill",
            "empire_mill",
            "empire_bakery",
            "empire_brewery",
            "empire_vineyard",
            "empire_winery",
            "empire_tavern",
            "empire_inn",
            "empire_charcoal_kiln",
            "empire_smelting_works",
            "empire_toolsmithy",
            "empire_armorsmithy",
            "empire_barracks"
         }
         local most = 1
         local selc = 0
         for idx,site in ipairs(bld) do
            if #p1:get_buildings(site) > most then
               most = #p1:get_buildings(site)
               local build = p1:get_buildings(site)
               selc = build[1]
            end
         end
         if selc ~= 0 then
            local fields = selc.fields
            local prior_center = scroll_to_field(fields[1])
            selc:destroy()
            campaign_message_box(amalea_16)
            scroll_to_map_pixel(prior_center)
         end
      end
   else
      for count = 0, 10 do
         sleep(1500000)
         local hq = p1:get_buildings("empire_headquarters")
         local beer = hq[1]:get_wares("beer") + 20
         local wine = hq[1]:get_wares("wine") + 10
         hq[1]:set_wares("beer", beer)
         hq[1]:set_wares("wine", wine)
         campaign_message_box(amalea_17)
      end
   end
end

function mission_thread()
   sleep(1000)
   scroll_to_field(sf)  --scroll to our headquarters
   include "map:scripting/starting_conditions.lua"
   sleep(1000)

   --Initial messages
   campaign_message_box(diary_page_1)
   sleep(700)
   campaign_message_box(saledus)
   sleep(700)
   campaign_message_box(amalea)

   -- the mayor is appearing
   sleep(700)
   campaign_message_box(lutius_1)
   sleep(700)
   campaign_message_box(marcus_1)
   sleep(700)
   campaign_message_box(lutius_2)
   sleep(700)
   campaign_message_box(marcus_2)
   sleep(700)
   campaign_message_box(lutius_3)
   sleep(700)

   -- let's start with dismantling the unproductive buildings
   campaign_message_box(amalea_1)
   run(dismantle)
   run(farm_plans)
end

run(mission_thread)
