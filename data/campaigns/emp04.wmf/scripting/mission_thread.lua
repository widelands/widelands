include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"

function dismantle()
   local o = add_campaign_objective(obj_dismantle_buildings)
   
   sleep(5000)
   while count_buildings(p1, {"empire_fishers_house", "empire_quarry", "empire_lumberjacks_house2", "empire_well2", "empire_farm1"}) > 0 do
   sleep(4000)
   end
   o.done = true
   campaign_message_box(amalea_3)
end

function farm_plans()
   while #p1:get_buildings("empire_farm1") > 1 do sleep(3249) end
   campaign_message_box(amalea_2)
   local o = add_campaign_objective(obj_find_farm_plans)
   
end   

function mission_thread()
   sleep(1000)
   scroll_to_field(sf)  --scroll to our headquarters
   include "map:scripting/starting_conditions.lua"
   

   --Initial messages
   campaign_message_box(diary_page_1)
   sleep(1000)
   campaign_message_box(saledus)
   sleep(1000)
   campaign_message_box(amalea)

   -- let's start with dismantling the unproductive buildings 
   sleep(1000)
   campaign_message_box(diary_page_2)
   sleep(1000)
   campaign_message_box(amalea_1)
   run(dismantle)
   run(farm_plans)

   -- sleep(400)

   -- sleep(400)
   -- campaign_message_box(saledus_1)
   -- o = add_campaign_objective(obj_build_first_outpost)


   -- -- Outpost is completed now
   -- campaign_message_box(saledus_2)

   -- 
   -- while #p1:get_buildings("empire_quarry") < 1 do sleep(3000) end
   -- o1.done = true

   -- -- quarry is now build but we need more basic infrastructure
   -- campaign_message_box(amalea_2)
   -- p1:allow_buildings{
      -- "empire_barrier", 
      -- "empire_sawmill", 
      -- "empire_stonemasons_house", 
      -- "empire_foresters_house", 
      -- "empire_tower",
   -- }
   -- p2:allow_buildings{
      -- "barbarians_tower", 
      -- "barbarians_fortress",
   -- }
   -- o2 = add_campaign_objective(obj_build_sawmill_stonemason_and_lumberjacks)
   -- -- in the same time we need to discover more land and a port space
   -- run(building_industry)
   -- sleep(40000)
   -- campaign_message_box(saledus_3)
   -- p1:allow_buildings{
      -- "empire_port", 
      -- "empire_shipyard",
   -- }
   -- o4 = add_campaign_objective(obj_find_port_space)

   -- local port = map:get_field(17, 17)
   -- local fowned = nil
   -- while not fowned do
      -- if port.owner == p1 then
         -- fowned = 1
         -- break
      -- end
   -- sleep(3000)
   -- end

   -- sleep(3213)
   -- o4.done = true

   -- -- Portspace discovered now we can build a port
   -- campaign_message_box(saledus_4)
   -- p1:allow_buildings{
      -- "empire_farm", 
      -- "empire_fishers_house", 
      -- "empire_hunters_house",
   -- }
   -- o5 = add_campaign_objective(obj_build_port_and_shipyard)
   -- run(ship_industry)

   -- -- patience we have to think about how to get enough cloth to build a ship
   -- campaign_message_box(amalea_4)
   -- o6 = add_campaign_objective(obj_produce_wheat)
   -- run(wheat) 
end

run(mission_thread)
-- run(check_military)
