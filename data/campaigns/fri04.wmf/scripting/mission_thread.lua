local seen_reebaud = false
local seen_amazons = false

local o_defeat_amz = nil
local o_defeat_emp = nil

function see_amazons(field)
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
   campaign_message_box(victory_amz)
   set_objective_done(o_defeat_amz)
end

function see_reebaud(field)
   scroll_to_field(field)
   sleep(2000)
   p2.hidden_from_general_statistics = false
   campaign_message_box(reebaud_1)
   p2.team = 1
   campaign_message_box(reebaud_2)
   campaign_message_box(reebaud_3)
   campaign_message_box(reebaud_4)
   sleep(2000)
   campaign_message_box(reebaud_5)
   campaign_message_box(reebaud_6)
   campaign_message_box(reebaud_7)
   campaign_message_box(reebaud_8)
   if seen_amazons then campaign_message_box(reebaud_9a) else campaign_message_box(reebaud_9b) end
   p3.team = 0
   if not o_defeat_amz then o_defeat_amz = add_campaign_objective(obj_defeat_amz) end
   if campaign_data.payment then
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
   sleep(5000)

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
   campaign_message_box(victory)
   p1:mark_scenario_as_solved("fri04.wmf")
   -- END OF MISSION 3
end

function mission_thread()
   campaign_message_box(intro_1)
   scroll_to_field(map.player_slots[1].starting_field)
   include "map:scripting/starting_conditions.lua"
   sleep(3000)
   if campaign_data.payment then campaign_message_box(intro_2) end
   sleep(2000)
   campaign_message_box(intro_3)
   local o = add_campaign_objective(obj_find_reebaud)
   local f
   while true do
      sleep(1000)
      for x=0,map.width - 1 do
         for y=0,map.height - 1 do
            f = map:get_field(x,y)
            sleep(5)
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
