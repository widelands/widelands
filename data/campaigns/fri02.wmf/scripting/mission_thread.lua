include "scripting/messages.lua"

local done_exp = false
local done_mine = false
local done_fight = false

local has_mountain = false
local see_empire = nil

local fields = {}
for x=0,map.width-1 do
   for y=0,map.height-1 do
      fields [#fields + 1] = map:get_field(x,y)
   end
end

function get_land (p)
   local land = 0
   for idx,f in ipairs (fields) do
      if f.owner == p then land = land + 1 end
   end
   return land
end

function check_mountain ()
   while true do
      sleep (7777)
      for idx,f in ipairs(fields) do
         if f:has_caps ("mine") and f.owner == p1 then 
            has_mountain = true
            return
         end
      end
   end
end

function check_empire ()
   while true do
      sleep (7777)
      for idx,f in ipairs(fields) do
         local p1c = false
         local p2c = false
         for idx,cl in ipairs(f.claimers) do
            if cl == p1 then p1c = true end
            if cl == p2 then p2c = true end
         end
         if p1c and p2c then 
            see_empire = f
            return
         end
      end
   end
end

function expand_south ()
   campaign_message_box (expand_s_1)
   local o = add_campaign_objective (obj_expand_south)
   run (check_empire)
   while see_empire == nil do sleep (777) end
   set_objective_done (o)

   scroll_to_field (see_empire)
   campaign_message_box (supply_murilius_1)
   campaign_message_box (supply_murilius_2)
   campaign_message_box (supply_murilius_3)
   campaign_message_box (supply_murilius_4)
   campaign_message_box (supply_murilius_5)
   local placed = false
   local radius = 1
   while not placed do
      radius = radius + 1
      for idx,f in ipairs (see_empire:region (radius, radius - 1)) do
         if f.owner == p1 and f:has_caps ("medium") and not placed then
            p1:place_building ("empire_warehouse", f)
            placed = true
         end
      end
   end
   local wh = p1:get_buildings ("empire_warehouse") [1]
   wh:set_warehouse_policies ("log", "prefer")
   wh:set_warehouse_policies ("granite", "prefer")
   wh:set_warehouse_policies ("water", "prefer")
   wh:set_warehouse_policies ("fish", "prefer")
   wh:set_warehouse_policies ("ration", "prefer")
   wh:set_warehouse_policies ("meal", "prefer")
   wh:set_warehouse_policies ("beer", "prefer")
   wh:set_warehouse_policies ("coal", "prefer")
   wh:set_warehouse_policies ("iron", "prefer")
   wh:set_warehouse_policies ("gold", "prefer")
   wh:set_warehouse_policies ("iron_ore", "prefer")
   wh:set_warehouse_policies ("gold_ore", "prefer")
   scroll_to_field (wh.fields [1])
   sleep (2000)
   campaign_message_box (supply_murilius_6)
   campaign_message_box (supply_murilius_7)
   campaign_message_box (supply_murilius_8)
   
   o = add_campaign_objective (obj_supply_murilius)
   local choice = nil
   local land = get_land (p1)
   while choice == nil do
      sleep (7777)
      if #(p1:get_buildings ("empire_warehouse")) < 1 then 
         choice = "destroy"
      elseif get_land (p1) > land then 
         choice = "military"
      else
         if wh:get_wares ("log") >= 30 and
            wh:get_wares ("granite") >= 40 and
            wh:get_wares ("water") >= 150 and
            wh:get_wares ("fish") >= 30 and
            wh:get_wares ("ration") >= 40 and
            wh:get_wares ("meal") >= 10 and
            wh:get_wares ("coal") >= 30 and
            wh:get_wares ("iron") >= 20 and
            wh:get_wares ("gold") >= 10 and
            wh:get_wares ("iron_ore") >= 40 and
            wh:get_wares ("gold_ore") >= 20 and
            wh:get_wares ("beer") >= 30 
         then
            choice = "yes"
         end
      end
   end
   set_objective_done (o)
   p3:allow_buildings ("all")
   done_exp = true
   if choice == "yes" then
      supply_yes ()
   else
      if choice == "military" then
         campaign_message_box (supply_murilius_military)
      else
         campaign_message_box (supply_murilius_destroy)
      end
      supply_no ()
   end
end

function mining_issues ()
   run (check_mountain)
   while not has_mountain do sleep (777) end
   campaign_message_box (train_recycle_1)
   campaign_message_box (train_recycle_2)
   campaign_message_box (train_recycle_3)
   local o = add_campaign_objective (obj_train_recycle)
   p1:allow_buildings {"frisians_recycling_centre"}
   local left = true
   local wares = {"scrap_metal_iron", "scrap_metal_mixed", "sword_basic", "sword_long", "sword_curved", "sword_double"}
   while left do 
      sleep (4273) 
      left = false
      local buildings = array_combine (
         p1:get_buildings ("frisians_port"),
         p1:get_buildings ("frisians_warehouse")
      )
      for idx,bld in ipairs (buildings) do
         for idy,ware in ipairs (wares) do
            if bld:get_wares (ware) > 0 then left = true end
         end
      end
      buildings = array_combine(
         p1:get_buildings ("frisians_recycling_centre"),
         p1:get_buildings ("frisians_training_camp"),
         p1:get_buildings ("frisians_training_arena")
      )
      for idx,bld in ipairs (buildings) do
         for idy,ware in ipairs (wares) do
            if bld:get_inputs (ware) > 0 then left = true end
         end
      end
   end
   set_objective_done (o)

   while not (check_for_buildings (p1, {frisians_coalmine = 1}) or check_for_buildings (p1, {frisians_ironmine = 1}) or check_for_buildings (p1, {frisians_goldmine = 1}) or check_for_buildings (p1, {frisians_rockmine = 1})) do sleep (4273) end
   
   campaign_message_box (aqua_farm_1)
   local o = add_campaign_objective (obj_aqua_farm)
   p1:allow_buildings {"frisians_aqua_farm"}
   while not check_for_buildings (p1, {frisians_aqua_farm = 1}) do sleep (4273) end
   set_objective_done (o)
   
   done_mine = true
end

function supply_yes ()
   local wh = p1:get_buildings ("empire_warehouse") [1]
   local hq = p2:get_buildings ("empire_headquarters") [1]
   for name,nb in ipairs (wh:get_wares ("all")) do
      if p2.tribe:has_ware (name) then 
         wh:set_wares (name, 0)
         hq:set_wares (name, hq:get_wares (name) + nb)
      end
   end
   campaign_message_box (supply_murilius_thanks)
   local o = add_campaign_objective (obj_defeat_barbarians)
   while not p3.defeated do sleep (4513) end
   set_objective_done (o)
   if not p2.defeated then 
      campaign_message_box (defeat_murilius_1)
      campaign_message_box (defeat_murilius_2)
      p2.team = 0
      p2:allow_buildings ("all")
      add_campaign_objective (obj_defeat_murilius)
      while get_land (p2) > 0 do 
         sleep (7777)
      end
   end
   
   done_fight = true
end

function supply_no ()
   p2.team = 0
   campaign_message_box (defeat_both)
   local o = add_campaign_objective (obj_defeat_both)
   while not p3.defeated do sleep (4829) end
   p2:allow_buildings ("all")
   while not p2.defeated do sleep (4829) end
   done_fight = true
end

function victory () --when all objectives are done
   while not done_exp do sleep (4731) end
   while not done_mine do sleep (4731) end
   while not done_fight do sleep (4731) end
   sleep (10000)
   campaign_message_box (victory_1)
   --END OF THE FRISIAN CAMPAIGN – no further scenarios
end

function mission_thread()
   
   scroll_to_field (map.player_slots [1].starting_field)
   campaign_message_box (intro_1)
   include "map:scripting/starting_conditions.lua"
   sleep (2000)
   
   p1.team = 1
   p2.team = 1
   p3.team = 0
   
-- TODO list
-- · tell p2 not to attack a building of p3 if its internal name begins with "barbarians_"
-- · p1 and p2 may not share a vision while allies
   
   campaign_message_box (intro_2)
   local o = add_campaign_objective (obj_new_home)
   while not check_for_buildings (p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 1, frisians_well = 1, frisians_reed_farm = 1, frisians_quarry = 1, frisians_brick_burners_house = 1, frisians_claypit = 1, frisians_charcoal_kiln = 1}) do sleep (4273) end
   set_objective_done (o)
   
   run (expand_south)
   run (mining_issues)
   run (victory)

end

run (mission_thread)
