include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"

-- Some objectives need to be waited for in separate threads
local obj_build_port_and_shipyard_done = false
local obj_find_artifacts_done = false

--  If more than 3 ships are available, it is time to explore the whole map
function explore_sea()
   while #p1:get_ships() < 3 do sleep(3000) end

   -- Check if all possible ports are built
   if #p1:get_buildings("empire_port") < 5 then
      campaign_message_box(amalea_11)
      local objective = add_campaign_objective(obj_find_all_ports, 3000)
      while #p1:get_buildings("empire_port") < 5 do sleep(3000) end
      objective.done = true
   end

   campaign_message_box(amalea_12)
   run(artifacts)
end

-- Check for control of all pieces of Neptune's shrine (artifacts)
function artifacts()
   -- Objective will be triggered if 50+ buildings are built
   local all_building_types = p1.tribe.buildings
   while count_buildings(p1, all_building_types) < 50 do
      sleep(4000)
   end
   campaign_message_box(saledus_8)

   local objective = add_campaign_objective(obj_find_artifacts)

   -- We send a slightly different message the first time
   local first_message = true
   local artifact_fields = get_artifact_fields()
   repeat
      for idx, f in ipairs(artifact_fields) do
         sleep(2000)
         if f.owner == p1 then
            -- We have found a new artifact
            local prior_center = scroll_to_field(f)
            sleep(1000)
            campaign_message_box(diary_page_6(first_message, #artifact_fields - 1))
            sleep(2000)
            if f.immovable then
               f.immovable:remove()
               sleep(2000)
            else
               print("Failed to remove artifact at (" .. f.x .. ", " .. f.y .. ")")
            end
            scroll_to_map_pixel(prior_center)
            first_message = false
            artifact_fields = get_artifact_fields()
         end
      end
   until #artifact_fields == 0

   campaign_message_box(saledus_9)
   objective.done = true
   obj_find_artifacts_done = true
end

-- Check for completed Woodcutter and Sawmill
function check_wood_industry()
   while not check_for_buildings(p1, { empire_lumberjacks_house = 1, empire_sawmill = 1}) do sleep(2000) end
   campaign_message_box(amalea_10)
end

-- Check for too many military buildings
function check_military()
   local number_mil_buildings = {
         "empire_sentry",
         "empire_blockhouse",
         "empire_outpost",
         "empire_barrier",
         "empire_tower",
   }
   local too_many_mil_buildings = false
   while not too_many_mil_buildings do
      if count_buildings_by_name(p1, number_mil_buildings) > 2 and #p1:get_buildings("empire_port") < 2 then
         too_many_mil_buildings = true
         break
      end
   sleep(3000)
   end
   campaign_message_box(amalea_9)
end

-- Once we have a stonemason's house, check for economy options of marble columns lowered to 4
function stonemason_and_marble_columns()
   while #p1:get_buildings("empire_stonemasons_house") < 1 do sleep(3000) end
   campaign_message_box(amalea_3)

   -- Don't use all your marble for columns
   local objective = add_campaign_objective(obj_lower_marble_column_demand)

   --- Check the headquarters' flag's economy
   local eco = sf.brn.immovable.economy
   while eco:ware_target_quantity("marble_column") ~= 4 do
      sleep(2434)
   end
   sleep(4000)
   objective.done = true
   campaign_message_box(amalea_8)
end

-- Check for completed building industry
function building_industry()
   local objective = add_campaign_objective(obj_build_sawmill_stonemason_and_lumberjacks)

   run(stonemason_and_marble_columns)
   run(check_wood_industry)

   while not check_for_buildings(p1, { empire_lumberjacks_house = 1, empire_sawmill = 1, empire_stonemasons_house = 1})
      do sleep(2343) end
   objective.done = true
end


-- Check for gold to be able to send out expeditions
function check_gold()
   local objective = add_campaign_objective(obj_produce_gold)
   while p1:get_produced_wares_count('gold') < 6 do sleep(2434) end
   campaign_message_box(amalea_7)
   objective.done = true
end

-- Check for port and shipyard
function ship_industry()
   local objective = add_campaign_objective(obj_build_port_and_shipyard)

   -- Check for port built to realize we need gold
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
   run(check_gold)

   -- Check for both port and shipyard
   while not check_for_buildings(p1, { empire_port = 1, empire_shipyard =1 }) do sleep(2434) end
   campaign_message_box(diary_page_4)
   objective.done = true
   obj_build_port_and_shipyard_done = true
end

-- Check for enough wheat in warehouses
function wheat()
   -- Patience, we have to think about how to get enough cloth to build a ship
   campaign_message_box(amalea_4)
   local objective = add_campaign_objective(obj_produce_wheat)

   while count_in_warehouses("wheat") < 50 do sleep(2434) end
   campaign_message_box(amalea_5)
   objective.done = true

   -- We need to turn the wheat into cloth for building ships
   objective = add_campaign_objective(obj_produce_cloth)
   p1:allow_buildings{
      "empire_well",
      "empire_sheepfarm",
      "empire_weaving_mill",
      "empire_charcoal_kiln",
      "empire_marblemine",
      "empire_marblemine_deep"
   }
   p2:allow_buildings{
      "barbarians_warehouse" ,
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
   while #p1:get_buildings("empire_weaving_mill") < 1 do sleep(3000) end
   campaign_message_box(amalea_13)
   while p1:get_produced_wares_count('cloth') < 6 do sleep(2434) end
   objective.done = true
   run(expedition)
end

function expedition()
   -- Now we should explore our little world
   while not obj_build_port_and_shipyard_done do sleep(2434) end
   campaign_message_box(saledus_5)
   local objective = add_campaign_objective(obj_build_expedition_ports)

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

   objective.done = true

   objective = add_campaign_objective(obj_conquer_all)
   run(explore_sea)
   run(soldiers)

   while not p2.defeated do sleep(2342) end
   objective.done = true

   -- Babarians defeated.
   campaign_message_box(saledus_7)

   -- To get home, we need all artifacts as well
   while not obj_find_artifacts_done do sleep(3000) end

   -- Sleep a while to have some time between the last objective done message and final victory
   sleep(25000)
   campaign_message_box(diary_page_5)

   p1:mark_scenario_as_solved("emp03")
end

-- After discovery of Barbarian ruins, we should hurry to build a full training capability
function soldiers()
   local ruins = map:get_field(86,82)
   local ruin_fortress = map:get_field(85,80)

   while not p1:sees_field(ruins) do sleep(3000) end
   scroll_to_field(ruin_fortress,5)
   sleep(500)
   random_reveal(p1, ruin_fortress:region(5), 1000)
   sleep(500)
   campaign_message_box(saledus_12)

   -- If we don't have enough training sites yet, add a message and objective to complete them.
   local training = p1:get_buildings {
      "empire_trainingcamp",
      "empire_barracks",
      "empire_arena",
      "empire_colosseum"
   }
   if not check_trainingsites() then
      campaign_message_box(saledus_11, 3000)
      local objective = add_campaign_objective(obj_training)
      while not check_trainingsites() do sleep(3000) end
      objective.done = true
   end
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

   -- Scroll to the place where the ship is finally stranded
   scroll_to_field(sf)
   -- Now we place the shipwreck headquarters and fill it with workers and wares
   include "map:scripting/starting_conditions.lua"
   p1:hide_fields(sf:region(13), true)
   concentric_reveal(p1, sf, 13, 100)
   campaign_message_box(diary_page_3)
   sleep(400)
   campaign_message_box(saledus)
   sleep(400)
   campaign_message_box(amalea)
   sleep(400)
   campaign_message_box(saledus_1)
   local objective = add_campaign_objective(obj_build_first_outpost)
   while #p1:get_buildings("empire_outpost") < 1 do sleep(3249) end
   objective.done = true

   -- Outpost is completed now
   campaign_message_box(saledus_2)
   objective = add_campaign_objective(obj_build_quarry)
   campaign_message_box(amalea_1)
   while #p1:get_buildings("empire_quarry") < 1 do sleep(3000) end
   objective.done = true

   -- Quarry is now built but we need more basic infrastructure
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

   -- At the same time, we need to discover more land and a port space
   run(building_industry)
   sleep(40000)
   campaign_message_box(saledus_3)
   p1:allow_buildings{
      "empire_port",
      "empire_shipyard",
   }
   objective = add_campaign_objective(obj_find_port_space)

   local port = map:get_field(17, 17)
   while port.owner ~= p1 do sleep(3000) end

   sleep(3213)
   objective.done = true

   -- Portspace discovered, now we can build a port
   campaign_message_box(saledus_4)
   p1:allow_buildings{
      "empire_farm",
      "empire_fishers_house",
      "empire_hunters_house",
   }

   run(ship_industry)
   run(wheat)
end

run(mission_thread)
run(check_military)
