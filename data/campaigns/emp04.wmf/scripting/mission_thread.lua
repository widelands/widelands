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
   run (clear_roads)
   run (quarries_lumberjacks)
end

function farm_plans()
   local f = map:get_field(7, 168)
   while #p1:get_buildings("empire_farm1") > 1 do sleep(3249) end
   campaign_message_box(amalea_2)
   local o = add_campaign_objective(obj_find_farm_plans)
   while not (f.owner == p1) do 
   sleep(4000)
   end
   if f.immovable then
      f.immovable:remove()
      sleep(2000)
   else
      print("Failed to remove artifact at (" .. f.x .. ", " .. f.y .. ")")
   end
   campaign_message_box(amalea_4)
   o.done = true
   p1:allow_buildings{"empire_farm"}
end  

function clear_roads()
   local o = add_campaign_objective(obj_clear_roads)
   local cleared = false
   local count
   
   while cleared == false do 
   cleared = true
   sleep (5000)
      for x=7, 35 do
         for y=180, 207 do
           local field = map:get_field(x,y)
		   if (field.immovable and field.immovable.descr.type_name == "flag" and field.immovable.building == nil) then
			  local numroads = 0
              for _ in pairs(field.immovable.roads) do numroads = numroads + 1 end
		        if numroads < 2 then
		        cleared = false
                end
		    end
	     end
         for y=0, 25 do
           local field = map:get_field(x,y)
		   if (field.immovable and field.immovable.descr.type_name == "flag" and field.immovable.building == nil) then
			  local numroads = 0
              for _ in pairs(field.immovable.roads) do numroads = numroads + 1 end
		        if numroads < 2 then
		        cleared = false
                end
		    end
	     end
	  end	  
   end 
   o.done = true
   campaign_message_box(amalea_6)
end



function quarries_lumberjacks()
   local o = add_campaign_objective(obj_build_quarries_and_lumberjacks)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 3, empire_quarry = 2}) do sleep(3000) end
   o.done = true
   campaign_message_box(amalea_5)
   run(produce_food)
end

function produce_food()
   local o = add_campaign_objective(obj_produce_fish)
   while count_in_warehouses(ration) < 3 do sleep(3000) end
   o.done = true 
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
