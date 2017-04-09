include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"

function check_stonemason()  -- check for completed stonemason
   while #p1:get_buildings("empire_stonemasons_house") < 1 do sleep(3000) end
   campaign_message_box(amalea_3)

   -- don't use all your marble for columns
   o3 = add_campaign_objective(obj_lower_marble_column_demand)
   run(economy_settings)
end

function explore_sea() --  if more than 3 ships are available it is time to explore the whole map
   while #p1:get_ships() < 3 do sleep(3000) end
   if #p1:get_buildings("empire_port") == 5 then
      run(artifacts)
   else
      campaign_message_box(amalea_11)
      o11 = add_campaign_objective(obj_find_all_ports)
      -- check if all ports possible are build
      while #p1:get_buildings("empire_port") < 5 do sleep(3000) end
      o11.done = true
      campaign_message_box(amalea_12)
      run(artifacts)
   end
end

function artifacts()  -- check for control of all artifacts
   local artifact_fields = {}
   local i = 1
   -- find all artifacts
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         local field = map:get_field(x,y)
         if field.immovable and field.immovable:has_attribute("artifact") then
            -- this assumes that the immovable has size small or medium, i.e. only occupies one field
            artifact_fields[i] = map:get_field(x,y)
            i = i + 1
         end
      end
   end

   while true do
      sleep(4000)  
      -- objective will be triggered if 50+ buildings are build
      local number_buildings = {
         "empire_headquarters_shipwreck", 
         "empire_warehouse", 
         "empire_port", 
         "empire_quarry", 
         "empire_lumberjacks_house", 
         "empire_foresters_house", 
         "empire_fishers_house", 
         "empire_hunters_house", 
         "empire_well", 
         "empire_scouts_house", 
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
         "empire_shipyard", 
         "empire_donkeyfarm", 
         "empire_farm", 
         "empire_piggery", 
         "empire_sheepfarm", 
         "empire_weaving_mill", 
         "empire_weaponsmithy", 
         "empire_marblemine", 
         "empire_marblemine_deep", 
         "empire_coalmine", 
         "empire_coalmine_deep", 
         "empire_ironmine", 
         "empire_ironmine_deep", 
         "empire_goldmine", 
         "empire_goldmine_deep", 
         "empire_arena", 
         "empire_colosseum", 
         "empire_trainingcamp", 
         "empire_blockhouse",
         "empire_sentry", 
         "empire_outpost", 
         "empire_barrier", 
         "empire_tower", 
         "empire_fortress", 
         "empire_castle",
      }
      if count_buildings(p1, number_buildings) > 50 then
         break
      end
      sleep(4000)
   end
   campaign_message_box(saledus_8)
   campaign_message_box(show_artifacts)

   o12 = add_campaign_objective(obj_find_artifacts)

   local artifacts_owner = {}
   repeat
      sleep(2000)
      local all_artifacts_found = true
      for idx, f in ipairs(artifact_fields) do
         if f.owner then
            if not artifacts_owner[f] then
               if f.owner ~= p1 then
                  artifacts_owner[f] = nil
                  all_artifacts_found = false
               else
                  artifacts_owner[f] = f.owner
                  sleep(2000)
                  campaign_message_box(diary_page_6)
                  scroll_to_field(f)
               end
            end
         else
            all_artifacts_found = false
         end
      end
   until all_artifacts_found	 
   campaign_message_box(saledus_9)
   o12.done = true  
end

function check_wood_industry()  -- check for completed Woddcutter and sawmill
   while not check_for_buildings(p1, { empire_lumberjacks_house = 1, empire_sawmill = 1}) do sleep(2000) end
   campaign_message_box(amalea_10)
end

function check_military()  -- check for too much military buildings
   local too_much_mil_build = nil
   while not too_much_mil_build do
      local number_mil_buildings = {
         "empire_sentry", 
         "empire_blockhouse", 
         "empire_outpost", 
         "empire_barrier", 
         "empire_tower",
      }
      if count_buildings(p1, number_mil_buildings) > 2 and #p1:get_buildings("empire_port") < 2 then
         too_much_mil_build = 1
         break
      end
   sleep(3000)
   end
   campaign_message_box(amalea_9)
end

function economy_settings()  -- check for economy options of marble column lowered to 4
   local flag_field = map:get_field(4, 7)
   local flag = flag_field.immovable
   local eco = flag.economy
   while eco:ware_target_quantity("marble_column") ~= 4 do 
	 sleep(2434) 
   end
   sleep(4000)
   o3.done = true 
   campaign_message_box(amalea_8)   
end

function building_industry()  -- check for completed building industry
   run(check_stonemason)
   run(check_wood_industry)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 1, empire_sawmill = 1, empire_stonemasons_house = 1})
      do sleep(2343) end
   o2.done = true
   end

function port_build()  -- check for port build to realize we need gold
   while not check_for_buildings(p1, {empire_port = 1}) do sleep(2434) end
   campaign_message_box(amalea_6)
   p1:allow_buildings{ 
      "empire_brewery",
      "empire_coalmine",
      "empire_coalmine_deep",
      "empire_ironmine",
      "empire_ironmine_deep",
      "empire_goldmine",
      "empire_goldmine_deep",
      "empire_vineyard",
      "empire_winery",
      "empire_mill",
      "empire_bakery",
      "empire_piggery",
      "empire_donkeyfarm",
      "empire_tavern",
      "empire_inn",
      "empire_toolsmithy",
      "empire_armorsmithy",
      "empire_weaponsmithy",
      "empire_smelting_works",
       }
   p2:allow_buildings{
      "barbarians_coalmine",
      "barbarians_ironmine",
      "barbarians_goldmine",
      "barbarians_granitemine",
      "barbarians_coalmine_deep",
      "barbarians_ironmine_deep",
      "barbarians_goldmine_deep",
      "barbarians_coalmine_deeper",
      "barbarians_ironmine_deeper",
      "barbarians_goldmine_deeper",
      "barbarians_port",
      "barbarians_cattlefarm",
      "barbarians_charcoal_kiln",
      "barbarians_weaving_mill",
      }
   o7 = add_campaign_objective(obj_produce_gold)
   run(check_gold)
end

function check_gold()  -- check for gold to be able to send out expeditions
   while p1:get_produced_wares_count('gold') < 6 do sleep(2434) end
   campaign_message_box(amalea_7)
   o7.done = true
end

function ship_industry()
   run(port_build)
   while not check_for_buildings(p1, { empire_port = 1, empire_shipyard =1 }) do sleep(2434) end
   campaign_message_box(diary_page_4)
   o5.done = true
end

function wheat() -- check for enough wheat in warehouses
   while count_in_warehouses("wheat") < 50 do sleep(2434) end
   campaign_message_box(amalea_5)
   p1:allow_buildings{"empire_well"}
   p2:allow_buildings{"barbarians_warehouse" }
   o6.done = true
   -- We need to turn the wheat into cloth for building ships
   o8 = add_campaign_objective(obj_produce_cloth)
   p1:allow_buildings{ 
      "empire_sheepfarm",
      "empire_weaving_mill",
      "empire_charcoal_kiln",
      "empire_marblemine",
      "empire_marblemine_deep"
   }
   p2:allow_buildings{
      "barbarians_port",
      "barbarians_shipyard", 
      "barbarians_brewery", 
      "barbarians_inn",
      "barbarians_smelting_works",
      "barbarians_metal_workshop",
      "barbarians_ax_workshop",
      "barbarians_warmill",
      "barbarians_helmsmithy",
      "barbarians_barracks",
      "barbarians_battlearena",
      "barbarians_trainingcamp",
      "barbarians_big_inn",
   }
   while p1:get_produced_wares_count('cloth') < 6 do sleep(2434) end
   o8.done = true
   run(expedition)
end

function expedition()
   -- now we should explore our little world
   while not o5.done == true do sleep(2434) end
   campaign_message_box(saledus_5)
   o9 = add_campaign_objective(obj_build_expedition_ports)

   while not check_for_buildings(p1, { empire_port = 3 }) do sleep(2434) end
   campaign_message_box(saledus_6)

   p1:allow_buildings{"empire_barracks",
                      "empire_fortress", 
                      "empire_arena", 
                      "empire_trainingcamp",
                      "empire_colosseum",
                      "empire_warehouse",
                      "empire_castle",
                      "empire_scouts_house",
   }
   p2:allow_buildings{"barbarians_scouts_hut",
                      "barbarians_citadel",
   }

   o9.done = true
   o10 = add_campaign_objective(obj_conquer_all)
   run(explore_sea)
   run(soldiers)

   while true do
      local barmil = {
         "barbarians_headquarters", 
         "barbarians_warehouse", 
         "barbarians_port"}
      if count_buildings(p2, barmil) < 1 then
         break
      end
      sleep(2342)
   end
   o10.done = true

   -- Babarians defeated. 
   campaign_message_box(saledus_7)

   while not o12.done == true do sleep(3000) end -- to get home we need as well all artifacts 
   sleep(25000) -- Sleep a while to have some time between the last objective done message and final victory
   campaign_message_box(diary_page_5)

   p1:reveal_campaign("campsect2")
   p1:reveal_scenario("empiretut03")
end

function soldiers() -- after discovery of babarian ruins we should hurry to build a full training capability
   local ruins = map:get_field(86,82)
   local ruin_fortress = map:get_field(85,80)

   while not p1:sees_field(ruins) == true do sleep(3000) end
   scroll_to_field(ruin_fortress,5)
   sleep(500)
   --p1: reveal_fields(ruin_fortress:region(5))
   random_reveal(p1, ruin_fortress:region(5), 1000)
   sleep(500)
   campaign_message_box(saledus_12)

   local training = p1:get_buildings{
      "empire_trainingcamp", 
      "empire_barracks", 
      "empire_arena", 
      "empire_colosseum"
   }

   if #training.empire_trainingcamp > 0 and 
      #training.empire_barracks > 0 and 
      (#training.empire_arena > 0 or #training.empire_colosseum > 0) then
      campaign_message_box(saledus_10)
   else
      campaign_message_box(saledus_11)
      o13 = add_campaign_objective(obj_training)
   end

   while not (#training.empire_trainingcamp > 0 and 
              #training.empire_barracks > 0 and 
              (#training.empire_arena > 0 or #training.empire_colosseum > 0)) do 
      sleep(3000) 
      training = p1:get_buildings{
         "empire_trainingcamp", 
         "empire_barracks", 
         "empire_arena", 
         "empire_colosseum"
      }
      end
   o13.done = true
   campaign_message_box(saledus_10)
end

function mission_thread()
   sleep(1000)

   -- Initial messages
   local sea = map:get_field(28, 8)
   scroll_to_field(sea,5)

   campaign_message_box(diary_page_1)

   -- Show the sea
   random_reveal(p1, sea:region(5), 1000)
   sleep(100)
   local ship = p1:place_ship(sea)
   sleep(2500)
   campaign_message_box(diary_page_2)

   -- Hide the sea after 2 seconds
   sleep(400)
   ship:remove()
   sleep(300)
   random_hide(p1, sea:region(6), 1000)
   sleep(300)

   -- Stranded again
   scroll_to_field(sf)  --scroll to the place where the ship is finally stranded
   include "map:scripting/starting_conditions.lua"  --now we place the shipwreck headquarters and fill it with workers and wares
   p1:hide_fields(sf:region(13), true)
   concentric_reveal(p1, sf, 13, 100)
   campaign_message_box(diary_page_3)
   sleep(400)
   campaign_message_box(saledus)
   sleep(400)
   campaign_message_box(amalea)
   sleep(400)
   campaign_message_box(saledus_1)
   o = add_campaign_objective(obj_build_first_outpost)
   while #p1:get_buildings("empire_outpost") < 1 do sleep(3249) end
   o.done = true

   -- Outpost is completed now
   campaign_message_box(saledus_2)
   o1 = add_campaign_objective(obj_build_quarry)
   campaign_message_box(amalea_1)
   while #p1:get_buildings("empire_quarry") < 1 do sleep(3000) end
   o1.done = true

   -- quarry is now build but we need more basic infrastructure
   campaign_message_box(amalea_2)
   p1:allow_buildings{
      "empire_barrier", 
      "empire_sawmill", 
      "empire_stonemasons_house", 
      "empire_foresters_house", 
      "empire_tower",
   }
   p2:allow_buildings{
      "barbarians_tower", 
      "barbarians_fortress",
   }
   o2 = add_campaign_objective(obj_build_sawmill_stonemason_and_lumberjacks)
   -- in the same time we need to discover more land and a port space
   run(building_industry)
   sleep(40000)
   campaign_message_box(saledus_3)
   p1:allow_buildings{
      "empire_port", 
      "empire_shipyard",
   }
   o4 = add_campaign_objective(obj_find_port_space)

   local port = map:get_field(17, 17)
   local fowned = nil
   while not fowned do
      if port.owner == p1 then
         fowned = 1
         break
      end
   sleep(3000)
   end

   sleep(3213)
   o4.done = true

   -- Portspace discovered now we can build a port
   campaign_message_box(saledus_4)
   p1:allow_buildings{
      "empire_farm", 
      "empire_fishers_house", 
      "empire_hunters_house",
   }
   o5 = add_campaign_objective(obj_build_port_and_shipyard)
   run(ship_industry)

   -- patience we have to think about how to get enough cloth to build a ship
   campaign_message_box(amalea_4)
   o6 = add_campaign_objective(obj_produce_wheat)
   run(wheat) 
end

run(mission_thread)
run(check_military)
