local seen_reebaud = false
local seen_amazons = false

local o_defeat_amz = nil
local o_defeat_emp = nil
local amazons_defeated_by_reebaud = false

function hurry_up()
   local did_warn = false
   while true do
      sleep(10000)
      if seen_reebaud or p4.defeated then return end
      if #p2:get_buildings("frisians_headquarters") < 1 then
         if p2.defeated then
            campaign_message_box(reebaud_defeated_1)
            wl.ui.MapView():close()
            return
         elseif not did_warn then
            campaign_message_box(reebaud_in_danger)
            did_warn = true
         end
      end
   end
end

function check_player_defeated(p, msg)
   while true do
      sleep(10000)
      if p.defeated then
         campaign_message_box(msg)
         wl.ui.MapView():close()
         return
      end
   end
end

function warn_empire_expansion()
   while (
         #p3:get_buildings("empire_blockhouse") +
         #p3:get_buildings("empire_sentry") +
         #p3:get_buildings("empire_outpost") +
         #p3:get_buildings("empire_barrier") +
         #p3:get_buildings("empire_tower") +
         #p3:get_buildings("empire_fortress") +
         #p3:get_buildings("empire_castle")) < 4 do
      sleep(100000)
      if seen_reebaud then return end
   end
   campaign_message_box(legate_expands)
end

function see_amazons(field)
   if amazons_defeated_by_reebaud then return end
   scroll_to_field(field)
   sleep(2000)
   if not seen_reebaud then
      campaign_message_box(amz_1a)
      campaign_message_box(amz_1b)
   else
      campaign_message_box(amz_1c)
   end
   sleep(500)
   campaign_message_box(amz_2)
   sleep(500)
   campaign_message_box(amz_3)
   if not o_defeat_amz then o_defeat_amz = add_campaign_objective(obj_defeat_amz) end
   while not p4.defeated do sleep(1000) end
   if seen_reebaud then campaign_message_box(victory_amz_a) else campaign_message_box(victory_amz_b) end
   set_objective_done(o_defeat_amz)
end

function see_reebaud(field)
   local o_recruit_train = nil
   scroll_to_field(field)
   sleep(2000)
   campaign_message_box(reebaud_1)
   p2.team = 1
   campaign_message_box(reebaud_2)
   campaign_message_box(reebaud_3)
   campaign_message_box(reebaud_4)
   sleep(2000)
   campaign_message_box(reebaud_5)
   campaign_message_box(reebaud_6)
   campaign_message_box(reebaud_7)
   if p4.defeated then  -- Amazons are already gone
      amazons_defeated_by_reebaud = true
      campaign_message_box(reebaud_8b)
      if not campaign_data.payment then  -- Empire doesn't exist
         campaign_message_box(reebaud_9c)
         o_recruit_train = add_campaign_objective(obj_recruit_train)
      end
   else  -- Need to fight the Amazons
      campaign_message_box(reebaud_8a)
      if seen_amazons then campaign_message_box(reebaud_9a) else campaign_message_box(reebaud_9b) end
   end
   if not o_defeat_amz and not amazons_defeated_by_reebaud then o_defeat_amz = add_campaign_objective(obj_defeat_amz) end
   if campaign_data.payment then  -- Need to fight the Empire
      sleep(2000)
      campaign_message_box(reebaud_10)
      campaign_message_box(reebaud_11)
      campaign_message_box(reebaud_12)
      o_defeat_emp = add_campaign_objective(obj_defeat_emp)
      p1:set_attack_forbidden(3, false)
      p2:set_attack_forbidden(3, false)
      p3:set_attack_forbidden(1, false)
      p3:set_attack_forbidden(2, false)
      while not p3.defeated do sleep(1000) end
      campaign_message_box(victory_emp)
      set_objective_done(o_defeat_emp)
   end
   while not p4.defeated do sleep(1000) end

   if o_recruit_train then
      sleep(1000 * 60 * 60)
      set_objective_done(o_recruit_train)
   else
      sleep(5000)
   end

   local data = {}
   for x = 0, map.width - 1 do
      for y = 0, map.height - 1 do
         for i,bob in pairs(map:get_field(x, y).bobs) do
            if bob.descr.name == "frisians_soldier" then
               local key = bob.health_level .. bob.attack_level .. bob.defense_level
               if data[key] then
                  data[key] = data[key] + 1
               else
                  data[key] = 1
               end
            end
         end
      end
   end
   game:save_campaign_data("frisians", "fri04", data)
   if o_recruit_train then campaign_message_box(victory_b) else campaign_message_box(victory_a) end
   p1:mark_scenario_as_solved("fri04.wmf")
   -- END OF MISSION 4
end

function mission_thread()
   p1:set_attack_forbidden(2, true)
   p1:set_attack_forbidden(3, true)
   p2:set_attack_forbidden(1, true)
   p2:set_attack_forbidden(3, true)
   p3:set_attack_forbidden(1, true)
   p3:set_attack_forbidden(2, true)
   p3:set_attack_forbidden(4, true)
   p4:set_attack_forbidden(3, true)

   campaign_message_box(intro_1)
   scroll_to_field(map.player_slots[1].starting_field)
   include "map:scripting/starting_conditions.lua"
   if campaign_data.payment then
      sleep(3000)
      campaign_message_box(intro_2)
   end
   sleep(2000)
   campaign_message_box(intro_3)
   local o = add_campaign_objective(obj_find_reebaud)
   local f
   run(check_player_defeated, p1, player_defeated)
   run(hurry_up)
   if campaign_data.payment then run(warn_empire_expansion) end
   while true do
      for x=0,map.width - 1 do
         sleep(50)
         for y=0,map.height - 1 do
            f = map:get_field(x,y)
            local p1c = false
            local p2c = false
            local p4c = false
            for idx,claimer in ipairs(f.claimers) do
               if claimer == p1 then p1c = true end
               if claimer == p2 then p2c = true end
               if claimer == p4 then p4c = true end
            end
            if p1c and p2c and not seen_reebaud then
               seen_reebaud = true
               p2.hidden_from_general_statistics = false
               set_objective_done(o)
               run(see_reebaud, f)
               run(check_player_defeated, p2, reebaud_defeated_2)
            end
            if p1c and p4c and not seen_amazons then
               seen_amazons = true
               p4.hidden_from_general_statistics = false
               run(see_amazons, f)
            end
            if seen_reebaud and seen_amazons then return end
         end
      end
   end
end

run(mission_thread)
