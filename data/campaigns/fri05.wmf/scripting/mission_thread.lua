function check_trade()
   while true do
      -- NOCOM
      sleep(10000)
   end
end

function mission_thread()
   include "map:scripting/init_p2.lua"
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
      scroll_to_field(f)
      sleep(750)
   end

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

   while not p2.defeated do sleep(1000) end

   -- NOCOM
end

run(mission_thread)
