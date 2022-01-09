function check_fish()
   -- Slowly replenish fish in fishable nodes where the fish is not entirely depleted.
   while true do
      for y=0,map.height-1 do
         for x=0,map.width-1 do
            local f = map:get_field(x, y)
            if f.resource == "resource_fish" and f.resource_amount > 0 and f.resource_amount < f.initial_resource_amount then
               f.resource_amount = f.resource_amount + 1
            end
         end
         sleep(75)
      end
   end
end

function check_trade()
   while true do
      for _,port in ipairs(p1:get_buildings("frisians_port")) do
         for _,proposal in ipairs(trade) do
            local trade_interesting = false
            for output,amount in pairs(proposal[2]) do
               if port.flag.ware_economy:needs(output) then
                  trade_interesting = true
                  break
               end
            end
            if trade_interesting then
               for input,amount in pairs(proposal[1]) do
                  if port:get_wares(input) < amount then
                     trade_interesting = false
                     break
                  end
               end
               if trade_interesting then
                  for input,amount in pairs(proposal[1]) do
                     port:set_wares(input, port:get_wares(input) - amount)
                  end
                  for output,amount in pairs(proposal[2]) do
                     port:set_wares(output, port:get_wares(output) + amount)
                  end
                  sleep(3600)
               end
            end
         end
      end
      sleep(7200)
   end
end

function mission_thread()
   include "map:scripting/init_p2.lua"
   run(check_fish)
   campaign_message_box(intro_1)
   campaign_message_box(intro_2)
   campaign_message_box(intro_3)
   campaign_message_box(intro_4)

   scroll_to_field(map.player_slots[1].starting_field)
   include "map:scripting/init_p1.lua"
   p1:reveal_fields(map.player_slots[2].starting_field:region(9))
   local reveal_fields = {}
   for x=0,121 do
      for y=17,240 do
         local f = map:get_field(x, y)
         p2:reveal_fields({f})
         if p1:sees_field(f) then
            p1:reveal_fields({f})  -- Make currently seen fields permanently visible without hiding them first
         else
            table.insert(reveal_fields, f)
         end
      end
   end
   run(function()
      reveal_randomly(p1, reveal_fields, 100)
      reveal_fields = nil
   end)
   sleep(5000)

   campaign_message_box(intro_5)
   campaign_message_box(intro_6)
   campaign_message_box(intro_7)

   scroll_to_field(map.player_slots[2].starting_field)
   sleep(10000)
   scroll_to_field(map.player_slots[1].starting_field)

   campaign_message_box(intro_8)
   campaign_message_box(intro_9)
   campaign_message_box(intro_10)
   local o = add_campaign_objective(obj_fight)
   sleep(1500)

   campaign_message_box(trade_1)
   campaign_message_box(trade_2)

   for i,f in ipairs({
      map:get_field(75, 107),
      map:get_field(55,  75),
      map:get_field(33,  72),
      map:get_field(65, 117),
      map:get_field(79, 161),
      map:get_field(89, 177),
   }) do
      sleep(750)
      scroll_to_field(f)
   end

   sleep(2500)
   campaign_message_box(trade_3)
   add_campaign_objective(obj_trade)
   run(check_trade)

   sleep(7500)
   scroll_to_field(map:get_field(78, 141))
   sleep(2500)
   campaign_message_box(getting_started_1)
   sleep(2500)
   scroll_to_field(map:get_field(86, 116))
   sleep(2500)
   campaign_message_box(getting_started_2)

   sleep(5000)
   scroll_to_field(map:get_field(90, 185))
   sleep(5000)
   campaign_message_box(getting_started_3)
   sleep(2500)
   scroll_to_field(map:get_field(79, 139))
   sleep(2500)
   campaign_message_box(getting_started_4)

   sleep(5000)
   scroll_to_field(map:get_field(95, 156))
   sleep(5000)
   campaign_message_box(getting_started_5)
   sleep(2500)
   scroll_to_field(map.player_slots[1].starting_field)
   sleep(2500)
   campaign_message_box(getting_started_6)

   local defeated = false
   while not defeated do
      sleep(3000)
      defeated = true
      for i,bld in ipairs(p2.tribe.buildings) do
         if #p2:get_buildings(bld.name) > 0 then
            defeated = false
            break
         end
      end
   end

   scroll_to_field(map.player_slots[2].starting_field)
   sleep(1500)
   campaign_message_box(victory_1)
   campaign_message_box(victory_2)
   campaign_message_box(victory_3)
   campaign_message_box(victory_4)
   campaign_message_box(victory_5)

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
   game:save_campaign_data("frisians", "fri05", data)
   p1:mark_scenario_as_solved("fri05.wmf")
   -- END OF MISSION 5
end

run(mission_thread)
