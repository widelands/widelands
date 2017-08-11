include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"




function mission_thread()
   sleep(1000)
   scroll_to_field(sf)  --scroll to our headquarters
   include "map:scripting/starting_conditions.lua"
   

   --Initial messages
   campaign_message_box(diary_page_1)
   campaign_message_box(saledus)
   campaign_message_box(amalea)


   -- campaign_message_box(diary_page_3)
   -- sleep(400)
   -- 
   -- sleep(400)
   -- campaign_message_box(amalea)
   -- sleep(400)
   -- campaign_message_box(saledus_1)
   -- o = add_campaign_objective(obj_build_first_outpost)
   -- while #p1:get_buildings("empire_outpost") < 1 do sleep(3249) end
   -- o.done = true

   -- -- Outpost is completed now
   -- campaign_message_box(saledus_2)
   -- o1 = add_campaign_objective(obj_build_quarry)
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
