include "scripting/messages.lua"

local done_exp = false
local done_mine = false
local done_fight = false

local has_mountain = nil
local see_empire = nil

local fields = {}
local mountains = {}
for x=0,map.width-1 do
   for y=0,map.height-1 do
      local f = map:get_field(x,y)
      fields [#fields + 1] = f
      if not (f.terd:find ("mountain") == nil) then mountains [#mountains + 1] = f end
   end
end

function count_military_buildings_p1 ()
   return (#p1:get_buildings ("frisians_sentinel") + 
      #p1:get_buildings ("frisians_outpost") + 
      #p1:get_buildings ("frisians_tower") + 
      #p1:get_buildings ("frisians_fortress") + 
      #p1:get_buildings ("frisians_wooden_tower") + 
      #p1:get_buildings ("frisians_wooden_tower_high"))
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
      for idx,f in ipairs(mountains) do
         if f:has_caps ("mine") and f.owner == p1 then 
            has_mountain = f
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
   while see_empire == nil do sleep (2221) end
   scroll_to_field (see_empire)
   campaign_message_box (supply_murilius_1)
   campaign_message_box (supply_murilius_2)
   campaign_message_box (supply_murilius_3)
   campaign_message_box (supply_murilius_4)
   campaign_message_box (supply_murilius_5)
   local placed = false
   local radius = 0
   while not placed do
      radius = radius + 1
      for idx,f in ipairs (see_empire:region (radius, radius - 1)) do
         if not placed then
            local suited = f:has_caps ("medium") and f.brn:has_caps ("flag")
            for idy,n in ipairs ({f, f.brn, f.rn, f.ln, f.tln, f.trn, f.bln, f.brn.brn, f.brn.bln, f.brn.rn}) do
               if not (n.owner == p1) then suited = false end
            end
            if suited then
               p1:place_building ("frisians_warehouse_empire", f, false, true)
               placed = true
            end
         end
      end
   end
   local wh = p1:get_buildings ("frisians_warehouse_empire") [1]
   for idx,ware in ipairs (p1.tribe.wares) do
      if p2.tribe:has_ware (ware.name) then
         wh:set_warehouse_policies (ware.name, "prefer")
      else
         wh:set_warehouse_policies (ware.name, "dontstock")
      end
   end
   scroll_to_field (wh.fields [1])
   sleep (1500)
   p2:forbid_buildings {"empire_fortress", "empire_barrier", "empire_outpost", "empire_tower"}
   campaign_message_box (supply_murilius_6)
   campaign_message_box (supply_murilius_7)
   campaign_message_box (supply_murilius_8)
   set_objective_done (o)
   
   o = add_campaign_objective (obj_supply_murilius)
   local choice = nil
   local milbld = count_military_buildings_p1 ()
   while choice == nil do
      sleep (2791)
      if #(p1:get_buildings ("frisians_warehouse_empire")) < 1 then 
         choice = "destroy"
      elseif count_military_buildings_p1 () > milbld then --it IS possible to destroy a military building and build a new one elsewhere
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
   p2:allow_buildings ("all")
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
   while has_mountain == nil do sleep (2221) end
   scroll_to_field (has_mountain)
   campaign_message_box (train_recycle_1)
   campaign_message_box (train_recycle_2)
   campaign_message_box (train_recycle_3)
   local o = add_campaign_objective (obj_train_recycle)
   p1:allow_buildings {"frisians_recycling_centre", "frisians_training_camp", "frisians_training_arena"}
   local miner = false
   while not miner do 
      sleep (4473) 
      local mines = array_combine(
         p1:get_buildings ("frisians_coalmine"),
         p1:get_buildings ("frisians_rockmine"),
         p1:get_buildings ("frisians_ironmine"),
         p1:get_buildings ("frisians_goldmine"),
         p1:get_buildings ("frisians_coalmine_deep"),
         p1:get_buildings ("frisians_rockmine_deep"),
         p1:get_buildings ("frisians_ironmine_deep"),
         p1:get_buildings ("frisians_goldmine_deep")
      )
      for idx,bld in ipairs (mines) do 
         if bld:get_workers ("frisians_miner") + bld:get_workers ("frisians_miner_master") > 0 then 
            miner = true 
         end 
      end
   end
   set_objective_done (o)
   campaign_message_box (aqua_farm_1)
   local o = add_campaign_objective (obj_aqua_farm)
   p1:allow_buildings {"frisians_aqua_farm", "frisians_furnace", "frisians_barracks", "frisians_seamstress"}
   while not check_for_buildings (p1, {frisians_aqua_farm = 1}) do sleep (4473) end
   set_objective_done (o)
   campaign_message_box (aqua_farm_2)
   p1:allow_buildings {"frisians_armour_smithy_small", "frisians_seamstress_master"}
   
   done_mine = true
end

function supply_yes ()
   local wh = p1:get_buildings ("frisians_warehouse_empire") [1]
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
      p2.team = 2
      add_campaign_objective (obj_defeat_murilius)
      while get_land (p2) > 0 do sleep (7777) end
   end
   
   done_fight = true
end

function supply_no ()
   p2.team = 2
   campaign_message_box (defeat_both)
   local o = add_campaign_objective (obj_defeat_both)
   while not (p2.defeated and p3.defeated) do sleep (4829) end
   done_fight = true
end

function victory ()
   while not (done_exp and done_mine and done_fight) do sleep (4731) end
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
   p3.team = 2
-- TODO: instead of alliances, just forbid certain players to attack each other:
--     · Beginning:          forbid 1/2, 2/1, 2/3
--     · Refusing alliance:  forbid only 2/3
--     · Accepting alliance: first unchanged, after p3 defeated: allow all
   
   campaign_message_box (intro_2)
   local o = add_campaign_objective (obj_new_home)
   while not check_for_buildings (p1, {frisians_woodcutters_house = 1, frisians_foresters_house = 1, frisians_well = 1, frisians_reed_farm = 1, frisians_quarry = 1, frisians_brick_burners_house = 1, frisians_claypit = 1, frisians_charcoal_kiln = 1}) do sleep (4273) end
   set_objective_done (o)
   
   run (expand_south)
   run (mining_issues)
   run (victory)

end

run (mission_thread)
