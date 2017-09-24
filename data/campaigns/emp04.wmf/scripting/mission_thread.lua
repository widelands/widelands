include "scripting/messages.lua"
include "map:scripting/helper_functions.lua"

-- Some objectives need to be waited for in separate threads
local obj_find_monastry_done = false



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
   local f = map:get_field(47, 190)
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
   run (wheat_chain)
end  

function clear_roads()
   local o = add_campaign_objective(obj_clear_roads)
   local cleared = false
   --local count
   
   while cleared == false do 
   cleared = true
   sleep (5000)
      for x=7, 40 do
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
         for y=0, 28 do
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

function no_trees()
   local trees = 100
   while trees > 8 do 
      trees = 0
      sleep (5000)
      for x=13, 25 do
         for y=182, 205 do
           local field = map:get_field(x,y)
		   if (field.immovable and field.immovable.descr.terrain_affinity) then
              trees = trees + 1 
		   end
	     end
      end
   end
   local o = add_campaign_objective(obj_replace_foresters)   
   campaign_message_box(amalea_7)
   while #p1:get_buildings("empire_foresters_house") < 2 do sleep(3249) end
   o.done = true
   campaign_message_box(amalea_8)
end

function quarries_lumberjacks()
   local o = add_campaign_objective(obj_build_quarries_and_lumberjacks)
   while not check_for_buildings(p1, { empire_lumberjacks_house = 3, empire_quarry = 2}) do sleep(3000) end
   o.done = true
   campaign_message_box(amalea_5)
   run(produce_food)
   run(no_trees)
end

function produce_food()
   local o = add_campaign_objective(obj_produce_fish)
   while p1:get_produced_wares_count("ration") < 14 do sleep(3000) end
   o.done = true
   run(steel)
end

function steel()
   campaign_message_box(amalea_13)
   local o = add_campaign_objective(obj_produce_tools)

   while not (( 
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
	  ) > 9) do 
   sleep(2500) 
   end
   
   o.done = true
   
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
   
   number_soldiers = number_soldiers + 9
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
	  if amount > number_soldiers then
	     enough_soldiers = true
      end
      sleep(4273)
   end
   o1.done = true
   campaign_message_box(saledus_6)
   run(training)
end

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
   
   strength = strength + 10
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
	  if amount > strength then
	     enough_strength = true
      end
      sleep(4273)
   end
   o.done = true
   p1:allow_buildings{"empire_trainingcamp", "empire_colosseum"}
   campaign_message_box(saledus_7)
   
   
   while not obj_find_monastry_done do sleep(3000) end
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
   
   run(conquer)
end

function conquer()
   campaign_message_box(saledus_9)
   local o = add_campaign_objective(obj_conquer_all)

   while not p2.defeated do sleep(2342) end
   objective.done = true

   -- Babarians defeated.
   campaign_message_box(saledus_10)

   -- Sleep a while to have some time between the last objective done message and final victory
   sleep(25000)
   campaign_message_box(diary_page_5)

   p1:reveal_campaign("campsect2")
   p1:reveal_scenario("empiretut04")


end

function wheat_chain()
   while not (p1:get_produced_wares_count('beer') > 4  and p1:get_produced_wares_count('flour') > 4) do sleep(2434) end
   local o = add_campaign_objective(obj_find_monastry)
   campaign_message_box(amalea_9)
   while not (p1:sees_field(map:get_field(16,156)) or p1:sees_field(map:get_field(16,157)) or p1:sees_field(map:get_field(17,158)) or p1:sees_field(map:get_field(17,159)) or p1:sees_field(map:get_field(18,160)) or p1:sees_field(map:get_field(18,161)) or p1:sees_field(map:get_field(19,162)) or p1:sees_field(map:get_field(20,162)) or p1:sees_field(map:get_field(21,162)) or p1:sees_field(map:get_field(22,162)) or p1:sees_field(map:get_field(23,162)) or p1:sees_field(map:get_field(24,162))) do sleep(2500) end
   local well = map:get_field(17, 154)
   place_building_in_region(p3, "empire_well", {map:get_field(17, 154)})
   local brew = map:get_field(19, 155)
   place_building_in_region(p3, "empire_brewery", {map:get_field(19, 155)})
   local mill = map:get_field(18, 156)
   place_building_in_region(p3, "empire_mill", {map:get_field(18, 156)})
   local ware = map:get_field(21, 158)
   place_building_in_region(p3, "empire_warehouse", {map:get_field(21, 158)})
   local sent = map:get_field(19, 157)
   place_building_in_region(p3, "empire_sentry", {map:get_field(19, 157)})
   o.done = true
   sleep(4000)
   local vesta = map:get_field(19, 157)
   local prior_center = scroll_to_field(vesta)
   concentric_reveal(p1, vesta, 7, 100)
   campaign_message_box(vesta_0)
   campaign_message_box(amalea_10)
   campaign_message_box(saledus_1)
   local o1 = add_campaign_objective(obj_deal_with_vesta)
   scroll_to_map_pixel(prior_center)
   
   local hq = p1:get_buildings("empire_headquarters")
   
   while not ((hq[1]:get_wares("wheat") > 34 and hq[1]:get_wares("wine") > 14) or p3.defeated) do sleep(4000) end
   if p3.defeated then
      o1.done = true
	  p1:allow_buildings{"empire_mill", "empire_brewery"}
      campaign_message_box(saledus_2)
	  campaign_message_box(vesta_2)
	  campaign_message_box(amalea_11)
	  campaign_message_box(saledus_4)
   else
      o1.done = true
	  local wh = p3:get_buildings("empire_warehouse")
	  wh[1]:set_workers("empire_carrier", 0)
	  local wheat = hq[1]:get_wares("wheat") - 35
	  local wine = hq[1]:get_wares("wine") - 15
	  hq[1]:set_wares("wheat", wheat)
	  hq[1]:set_wares("wine", wine)	  
	  p1:allow_buildings{"empire_mill", "empire_brewery"}
      campaign_message_box(vesta_1) 

      well.immovable:remove()
      brew.immovable:remove()
      mill.immovable:remove()
      ware.immovable:remove()
      sent.immovable:remove()
      place_building_in_region(p1, "empire_well", {map:get_field(17, 154)})
      place_building_in_region(p1, "empire_brewery", {map:get_field(19, 155)})
      place_building_in_region(p1, "empire_mill", {map:get_field(18, 156)})
      place_building_in_region(p1, "empire_warehouse", {map:get_field(21, 158)}, {wares = {water = 30, flour = 30, beer = 40,}})
      place_building_in_region(p1, "empire_sentry", {map:get_field(19, 157)})   
      campaign_message_box(amalea_12)
      campaign_message_box(saledus_3)
   end
   obj_find_monastry_done = true
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
   




end

run(mission_thread)

