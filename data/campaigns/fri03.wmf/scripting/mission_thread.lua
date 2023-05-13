include "scripting/field_animations.lua"

function insert_soldiers(tbl, what)
   for descr,n in pairs(what) do
      local key = descr[1] .. descr[2] .. descr[3]
      if tbl[key] then
         tbl[key] = tbl[key] + n
      else
         tbl[key] = n
      end
   end
end

function port_space_blocked(f)
   if f.owner == p1 then return false end
   for i,field in pairs({
      f,
      f.ln,
      f.ln.ln,
      f.rn,
      f.brn,
      f.brn.rn,
      f.bln,
      f.bln.ln,
      f.brn.brn,
      f.brn.bln,
      f.tln,
      f.tln.ln,
      f.trn,
      f.trn.rn,
      f.trn.trn,
      f.trn.tln,
      f.tln.tln,
   }) do
      if field.owner ~= nil then return true end
   end
   return false
end

function wait_for_port_or_lose(field)
   while not (field.immovable and field.immovable.descr.name == "frisians_port") do
      sleep(2341)
      if port_space_blocked(field) then
         while not p1:sees_field(field) do sleep(357) end
         campaign_message_box(lost_port_space_blocked)
         wl.ui.MapView():close()
         return
      end
   end
end

function mission_thread()

   p2.hidden_from_general_statistics = true
   p3.hidden_from_general_statistics = true

   scroll_to_field(map.player_slots[1].starting_field)
   include "map:scripting/starting_conditions.lua"

   p3:set_attack_forbidden(1, true)
   run(function()
      repeat
         sleep(2000)
      until
         #p1:get_buildings("empire_sentry") +
         #p1:get_buildings("empire_tower") +
         #p1:get_buildings("empire_barrier") +
         #p1:get_buildings("empire_outpost") +
         #p1:get_buildings("empire_blockhouse") +
         #p1:get_buildings("empire_fortress") +
         #p1:get_buildings("empire_castle") > 0
      p3:set_attack_forbidden(1, false)
   end)

   -- Introduction
   sleep(1000)
   campaign_message_box(intro_1)
   sleep(2000)
   campaign_message_box(intro_2)
   campaign_message_box(intro_3)
   sleep(1000)
   campaign_message_box(intro_4)
   sleep(500)
   campaign_message_box(intro_5)
   sleep(1000)
   campaign_message_box(intro_6)
   -- This objective stays active until the player meets the Empire
   local o_north = add_campaign_objective(obj_north)

   sleep(20 * 60 * 1000)
   campaign_message_box(diker)
   p1:allow_buildings({"frisians_dikers_house"})

   -- Wait until the first port space is found (and reachable)
   while true do
      sleep(2341)
      local ok = true
      for i,f in pairs(port_south:region(2)) do
         if f.owner ~= p1 then
            ok = false
            break
         end
      end
      if ok then
         ok = nil
         for x = 7, 12 do
            for y = 456, 470 do
               local f = map:get_field(x, y)
               if f.immovable and f.immovable.descr.type_name == "flag" and
                     f.immovable:get_distance(map.player_slots[1].starting_field.brn.immovable) then
                  ok = f
               end
            end
            if ok then break end
         end
         if ok then break end
      end
   end
   p1:conquer(port_south, 4)
   scroll_to_field(port_south)
   campaign_message_box(port_1)

   -- Wait for the player to build a port on the Volcano Island
   wait_for_port_or_lose(port_volcano)
   scroll_to_field(port_volcano)
   sleep(2000)
   campaign_message_box(port_2)

   -- Wait for the player to meet the Atlanteans
   local fields = {}
   for x = 0, map.width - 1 do
      for y = 307, 404 do
         table.insert(fields, map:get_field(x, y))
      end
   end
   local see_other = nil
   while not see_other do
      for i,f in ipairs(fields) do
         sleep(5)
         local p1c = false
         local p2c = false
         for j,claimer in ipairs(f.claimers) do
            if claimer == p1 then p1c = true end
            if claimer == p2 then p2c = true end
         end
         if p1c and p2c then
            see_other = f
            break
         end
      end
   end
   p2.hidden_from_general_statistics = false
   scroll_to_field(see_other)
   sleep(1000)
   campaign_message_box(atl_1)
   campaign_message_box(atl_2)
   campaign_message_box(atl_3)
   campaign_message_box(atl_4)
   campaign_message_box(atl_5)
   campaign_message_box(atl_6)
   local o = add_campaign_objective(obj_atl)

   -- Wait for the player to defeat the Atlanteans
   while not (map.player_slots[2].starting_field.immovable and
         map.player_slots[2].starting_field.immovable.descr.name == "frisians_port") do sleep(2341) end
   set_objective_done(o)
   scroll_to_field(map.player_slots[2].starting_field)
   campaign_message_box(atl_7)

   -- Wait for the player to build a port on the Ice-Desert Island
   wait_for_port_or_lose(port_desert_s)
   scroll_to_field(port_desert_s)
   sleep(1000)
   campaign_message_box(port_3)
   campaign_message_box(port_4)

   -- Wait for the player to find the other port space on the Ice-Desert Island
   while port_desert_n.owner ~= p1 do sleep(2341) end
   p1:conquer(port_desert_n, 4)
   scroll_to_field(port_desert_n)
   sleep(2000)
   campaign_message_box(port_5)

   -- Wait for the player to build a port on the Northern Shore
   wait_for_port_or_lose(port_north)
   scroll_to_field(port_north)
   sleep(1000)
   campaign_message_box(port_6)
   campaign_message_box(port_7)

   -- Wait for the player to meet the Empire
   fields = {}
   for x = 0, map.width - 1 do
      for y = 0, 164 do
         table.insert(fields, map:get_field(x, y))
      end
   end
   local see_other = nil
   while not see_other do
      for i,f in ipairs(fields) do
         sleep(5)
         local p1c = false
         local p3c = false
         for j,claimer in ipairs(f.claimers) do
            if claimer == p1 then p1c = true end
            if claimer == p3 then p3c = true end
         end
         if p1c and p3c then
            see_other = f
            break
         end
      end
   end
   set_objective_done(o_north)
   p3.hidden_from_general_statistics = false
   scroll_to_field(see_other)
   sleep(1000)
   local cost = {100, 200, 300}
   cost = p1:get_wares("gold") + cost[difficulty]
   campaign_message_box(emp_1)
   campaign_message_box(emp_2)
   campaign_message_box(emp_3)
   campaign_message_box(emp_4)
   campaign_message_box(emp_5)
   campaign_message_box(emp_6(cost))
   campaign_message_box(emp_7(cost))
   o = add_campaign_objective(obj_emp(cost))

   local end_time = game.time + 7200000 -- 2 hours until timeout
   while game.time < end_time do
      sleep(1999)
      local fight = nil
      if map.player_slots[3].starting_field.immovable and
            map.player_slots[3].starting_field.immovable.descr.name == "frisians_port" then
         fight = { payment = nil }
      elseif port_north.immovable:get_wares("gold") >= cost then
         fight = { payment = cost }
         p1:reveal_fields(map.player_slots[3].starting_field:region(7))
      end
      if fight then
         set_objective_done(o)
         -- Gather the soldiers. We take all who are on the northern island, no
         -- matter whether they are in warehouses, milsites, or walking around.
         fight.soldiers = {}
         for x = 0, map.width - 1 do
            for y = 24, 166 do
               for i,bob in pairs(map:get_field(x, y).bobs) do
                  if bob.descr.name == "frisians_soldier" then
                     insert_soldiers(fight.soldiers, {
                        [{ bob.health_level, bob.attack_level, bob.defense_level, 0 }] = 1
                     })
                  end
               end
            end
         end
         -- The campaign data contains two values:
         -- · payment: is nil if the Empire was conquered, else the amount of gold paid
         -- · soldiers: a table of all soldiers we take with us (in the usual syntax convention)
         game:save_campaign_data("frisians", "fri03", fight)
         scroll_to_field(map.player_slots[3].starting_field)
         local field = map.player_slots[3].starting_field.trn.trn.tln
         if fight.payment == nil then
            p1:place_ship(field)
            sleep(2000)
            campaign_message_box(victory_fight)
         else
            p3:place_ship(field)
            sleep(2000)
            campaign_message_box(victory_pay)
         end
         p1:mark_scenario_as_solved("fri03.wmf")
         -- END OF MISSION 3
         return
      end
   end
   -- Timeout
   if map.player_slots[3].starting_field.immovable and map.player_slots[3].starting_field.immovable.descr.name == "empire_port_large" then
      campaign_message_box(timeout_1)
   else
      campaign_message_box(timeout_2)
   end
   wl.ui.MapView():close()

end

run(mission_thread)
