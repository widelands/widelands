local done_mining = false
local done_fighting = false

local see_empire = nil

local all_fields = {}
local mountains = {}
local useful_fields = {}
for x=0,map.width - 1 do
   for y=0,map.height - 1 do
      local field = map:get_field(x, y)
      if field.terd:find("mountain") ~= nil then
         table.insert(mountains, field)
      end
      if field:has_caps("small") or field:has_caps("mine") then
         table.insert(useful_fields, field)
      end
      table.insert(all_fields, field)
   end
end

function count_military_buildings_p1()
   return
      #p1:get_buildings("frisians_sentinel") +
      #p1:get_buildings("frisians_outpost") +
      #p1:get_buildings("frisians_tower") +
      #p1:get_buildings("frisians_fortress") +
      #p1:get_buildings("frisians_wooden_tower") +
      #p1:get_buildings("frisians_wooden_tower_high")
end

function check_empire()
   while true do
      sleep(1000)
      for idx,field in ipairs(all_fields) do
         sleep(5)
         local p1c = false
         local p2c = false
         for idx,claimer in ipairs(field.claimers) do
            if claimer == p1 then p1c = true end
            if claimer == p2 then p2c = true end
         end
         if p1c and p2c then
            see_empire = field
            return
         end
      end
   end
end

function expand_south()
   campaign_message_box(expand_south_1)
   local o = add_campaign_objective(obj_expand_south)
   run(check_empire)
   while see_empire == nil do sleep(2221) end
   scroll_to_field(see_empire)
   campaign_message_box(supply_murilius_1)
   campaign_message_box(supply_murilius_2)
   campaign_message_box(supply_murilius_3)
   campaign_message_box(supply_murilius_4)
   campaign_message_box(supply_murilius_5)
   p2.hidden_from_general_statistics = false
   p3.hidden_from_general_statistics = false
   local placed = false
   local radius = 0
   while not placed do
      radius = radius + 1
      for idx,f in ipairs(see_empire:region(radius, radius - 1)) do
         if not placed then
            local suited = f:has_caps("medium") and f.brn:has_caps("flag")
            for idy,n in ipairs({f, f.brn, f.rn, f.ln, f.tln, f.trn, f.bln, f.brn.brn, f.brn.bln, f.brn.rn}) do
               if n.owner ~= p1 then
                  suited = false
                  break
               end
            end
            if suited then
               p1:place_building("frisians_warehouse_empire", f, false, true)
               placed = true
            end
         end
      end
   end
   local wh = p1:get_buildings("frisians_warehouse_empire")[1]
   for idx,ware in ipairs(p1.tribe.wares) do
      if p2.tribe:has_ware(ware.name) then
         wh:set_warehouse_policies(ware.name, "prefer")
      else
         wh:set_warehouse_policies(ware.name, "dontstock")
      end
   end
   scroll_to_field(wh.fields[1])
   sleep(3000)
   p2:forbid_buildings {"empire_fortress", "empire_castle", "empire_blockhouse", "empire_tower"}
   p3:forbid_buildings {"barbarians_citadel", "barbarians_tower"}
   campaign_message_box(supply_murilius_6)
   campaign_message_box(supply_murilius_7)
   campaign_message_box(supply_murilius_8)
   p2:allow_buildings {"empire_barracks"}
   p3:allow_buildings {"barbarians_barracks"}
   sleep(2000)
   campaign_message_box(supply_murilius_9)
   set_objective_done(o)

   o = add_campaign_objective(obj_supply_murilius)
   local o_scout = add_campaign_objective(obj_scout)
   local o_poem = nil
   local choice = ""
   sleep(30000) -- give the player some time to account for nearly completed buildings
   local milbld = count_military_buildings_p1()
   local scout = nil
   while choice == "" do
      sleep(791)

      local milsites = count_military_buildings_p1()
      if o_poem and milsites < milbld then
         -- the player has dismantled a militarysite, so we assume he understood the poem
         set_objective_done(o_poem)
         o_poem = nil
      end

      if o_scout and not scout then
         -- let's see if a scout is spying in Murilius's territory
         for i,house in pairs(p1:get_buildings("frisians_scouts_house")) do
            for j,field in pairs(house.fields[1]:region(17)) do -- the scout has a radius of 15
               for k,bob in pairs(field.bobs) do
                  if bob.descr.name == "frisians_scout" and
                        field.owner == p2 and
                        field.brn.owner == p2 and
                        field.bln.owner == p2 and
                        field.trn.owner == p2 and
                        field.tln.owner == p2 and
                        field.rn.owner == p2 and
                        field.ln.owner == p2 then
                     scout = bob
                     break
                  end
               end
               if scout then break end
            end
            if scout then break end
         end
      elseif o_scout and scout then
         if scout.field.immovable and scout.field.immovable.descr.name == "frisians_scouts_house" then
            set_objective_done(o_scout)
            campaign_message_box(expansion_hint)
            o_poem = add_campaign_objective(obj_poem)
            o_scout = nil
            scout = nil
         end
      end

      if #(p1:get_buildings("frisians_warehouse_empire")) < 1 then
         choice = "destroy"
      elseif milsites > milbld then
         -- It *is* permitted to destroy/dismantle a military building and build a new one elsewhere
         choice = "military"
      else
         if (wh:get_wares("log") >= 30 and
            wh:get_wares("granite") >= 40 and
            wh:get_wares("water") >= 150 and
            wh:get_wares("fish") >= 30 and
            wh:get_wares("ration") >= 40 and
            wh:get_wares("meal") >= 10 and
            wh:get_wares("coal") >= 30 and
            wh:get_wares("iron") >= 20 and
            wh:get_wares("gold") >= 10 and
            wh:get_wares("iron_ore") >= 40 and
            wh:get_wares("gold_ore") >= 20 and
            wh:get_wares("beer") >= 30)
         then
            choice = "supply"
         end
      end
   end
   -- We don’t need the scout/poem objectives anymore
   if o_scout then
      set_objective_done(o_scout)
   end
   if o_poem then
      set_objective_done(o_poem)
   end
   set_objective_done(o)
   p2:allow_buildings("all")
   p3:allow_buildings("all")
   -- We're done expanding, so we can start checking for victory
   run(victory)

   if choice == "supply" then
      supply_murilius()
   else
      if choice == "military" then
         campaign_message_box(supply_murilius_military)
      else
         campaign_message_box(supply_murilius_destroy)
      end
      defeat_murilius()
   end
end

function mining_issues()
   -- Wait until we have conquered a mountain
   local has_mountain = nil
   while has_mountain == nil do
      sleep(7777)
      for idx,field in ipairs(mountains) do
         if field:has_caps("mine") and field.owner == p1 then
            has_mountain = field
            break
         end
      end
   end
   sleep(2221)

   -- Now we need to recycle metal so we can get picks
   scroll_to_field(has_mountain)
   campaign_message_box(train_recycle_1)
   campaign_message_box(train_recycle_2)
   campaign_message_box(train_recycle_3)
   local o = add_campaign_objective(obj_train_recycle)
   if not (has_gold > 0) then campaign_message_box(train_recycle_4) end
   p1:allow_buildings {"frisians_recycling_center", "frisians_training_camp", "frisians_training_arena"}
   local has_miner = false
   while not has_miner do
      sleep(4473)
      local mines = array_combine(
         p1:get_buildings("frisians_coalmine"),
         p1:get_buildings("frisians_rockmine"),
         p1:get_buildings("frisians_ironmine"),
         p1:get_buildings("frisians_goldmine"),
         p1:get_buildings("frisians_coalmine_deep"),
         p1:get_buildings("frisians_rockmine_deep"),
         p1:get_buildings("frisians_ironmine_deep"),
         p1:get_buildings("frisians_goldmine_deep")
      )
      for idx,bld in ipairs(mines) do
         if bld:get_workers("frisians_miner") + bld:get_workers("frisians_miner_master") > 0 then
            has_miner = true
            break
         end
      end
   end
   set_objective_done(o)

   -- The mines are working – now provide fish
   campaign_message_box(aqua_farm_1)
   sleep(10000)
   campaign_message_box(aqua_farm_2)
   local o = add_campaign_objective(obj_aqua_farm)
   p1:allow_buildings {"frisians_aqua_farm", "frisians_furnace", "frisians_sewing_room"}
   while not check_for_buildings(p1, {frisians_aqua_farm = 1}) do sleep(4473) end
   set_objective_done(o)
   campaign_message_box(aqua_farm_3)
   campaign_message_box(aqua_farm_4)
   p1:allow_buildings {"frisians_armor_smithy_small", "frisians_tailors_shop", "frisians_barracks"}
   done_mining = true
end

function supply_murilius()
   local wh = p1:get_buildings("frisians_warehouse_empire")[1]
   local hq = p2:get_buildings("empire_headquarters")[1]

   -- transfer all wares that the Frisians and the Empire have in common
   for idx,ware in ipairs(p1.tribe.wares) do
      if p2.tribe:has_ware(ware.name) then
         local amount = wh:get_wares(ware.name)
         wh:set_wares(ware.name, 0)
         hq:set_wares(ware.name, hq:get_wares(ware.name) + amount)
      end
   end
   campaign_message_box(supply_murilius_thanks)
   local o = add_campaign_objective(obj_defeat_barbarians)
   while not p3.defeated do sleep(4513) end
   set_objective_done(o)

   -- If the Barbarians already defeated Murilius – well done.
   -- Otherwise, Murilius provokes Reebaud into ordering the player to conquer his entire colony
   -- (merely defeating the Empire isn’t enough).
   -- We don't bother to check water, walkable-only and other useless terrains.
   -- That would be really too much to ask from our poor player, now wouldn't it?
   if not p2.defeated then
      campaign_message_box(defeat_murilius_1)
      campaign_message_box(defeat_murilius_2)
      p1:set_attack_forbidden(2, false)
      p2:set_attack_forbidden(1, false)
      o = add_campaign_objective(obj_defeat_murilius)
      local def = false
      while not def do
         sleep(1000)
         def = true
         for idx,field in ipairs(useful_fields) do
            if field.owner == p2 then
               def = false
               break
            end
            sleep(5)
         end
      end
      set_objective_done(o)
   end
   done_fighting = true
end

function defeat_murilius()
   p1:set_attack_forbidden(2, false)
   p2:set_attack_forbidden(1, false)
   campaign_message_box(defeat_both)
   local o = add_campaign_objective(obj_defeat_both)
   while not (p2.defeated and p3.defeated) do sleep(4829) end
   set_objective_done(o)
   done_fighting = true
end

function victory()
   while not (done_mining and done_fighting) do sleep(4731) end
   sleep(10000)
   campaign_message_box(victory_1)
   p1:mark_scenario_as_solved("fri02.wmf")
   --END OF MISSION 2
end

function mission_thread()

   p2.hidden_from_general_statistics = true
   p3.hidden_from_general_statistics = true

   campaign_message_box(intro_1)
   p3:conquer(p1_start, 8)
   place_building_in_region(p3, "barbarians_farm", p1_start:region(2))
   place_building_in_region(p3, "barbarians_quarry", p1_start:region(7))
   place_building_in_region(p3, "barbarians_lumberjacks_hut", p1_start:region(7))
   place_building_in_region(p3, "barbarians_wood_hardener", p1_start:region(7))
   place_building_in_region(p3, "barbarians_metal_workshop", p1_start:region(7))
   place_building_in_region(p3, "barbarians_well", p1_start:region(7))
   place_building_in_region(p3, "barbarians_rangers_hut", p1_start:region(7))
   reveal_concentric(p1, p1_start, 10, true, 100)
   scroll_to_field(p1_start)
   sleep(2000)
   campaign_message_box(intro_2)
   include "map:scripting/starting_conditions.lua"
   sleep(5000)

   p1:set_attack_forbidden(2, true)
   p2:set_attack_forbidden(1, true)
   p2:set_attack_forbidden(3, true)
   run(function()
      repeat
         local conquered = (#p3:get_buildings("empire_sentry") +
                            #p3:get_buildings("empire_blockhouse") +
                            #p3:get_buildings("empire_outpost") +
                            #p3:get_buildings("empire_barrier") +
                            #p3:get_buildings("empire_tower") +
                            #p3:get_buildings("empire_fortress") +
                            #p3:get_buildings("empire_castle") -
                            #p2:get_buildings("barbarians_sentry") -
                            #p2:get_buildings("barbarians_barrier") -
                            #p2:get_buildings("barbarians_tower") -
                            #p2:get_buildings("barbarians_fortress") -
                            #p2:get_buildings("barbarians_citadel"))
         p2:set_attack_forbidden(3, conquered <= 0)
         sleep(6913)
      until p3.defeated or p2.defeated
      p2:set_attack_forbidden(3, false)
   end)

   campaign_message_box(intro_3)
   local o = add_campaign_objective(obj_new_home)
   while not check_for_buildings(p1, {
      frisians_woodcutters_house = 1,
      frisians_foresters_house = 1,
      frisians_well = 1,
      frisians_reed_farm = 1,
      frisians_quarry = 1,
      frisians_brick_kiln = 1,
      frisians_clay_pit = 1,
      frisians_charcoal_kiln = 1}) do sleep(4273) end
   set_objective_done(o)

   run(expand_south)
   run(mining_issues)

end

run(mission_thread)
