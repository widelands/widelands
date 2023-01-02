function count_in_warehouses(ware)
   local whs = array_combine(
      p1:get_buildings("frisians_headquarters"),
      p1:get_buildings("frisians_warehouse"),
      p1:get_buildings("frisians_port")
   )
   local rv = 0
   for idx,wh in ipairs(whs) do
      rv = rv + wh:get_wares(ware)
   end
   return rv
end

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
   local trade_count = 0
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
                  trade_count = tradecount + 1
                  if trade_count == 3 then
                     local prior_center = scroll_to_field(map:get_field(90, 132))
                     sleep(1000)
                     campaign_message_box(training)
                     scroll_to_map_pixel(prior_center)
                     prior_center = nil
                  end
                  sleep(3600)
               end
            end
         end
      end
      sleep(7200)
   end
end

function check_objective_wood()
   local o = add_campaign_objective(obj_wood)
   while not check_for_buildings(p1, {frisians_woodcutters_house = 3, frisians_foresters_house = 4}) do
      sleep(4273)
   end
   set_objective_done(o)
   campaign_message_box(secured_wood)
   while not count_in_warehouses("log") > 80 do
      sleep(4887)
   end
   local prior_center = scroll_to_field(map:get_field(95, 156))
   sleep(1000)
   campaign_message_box(diking)
   scroll_to_map_pixel(prior_center)
   prior_center = nil
end

function check_objective_block()
   local o = add_campaign_objective(obj_block)
   while true do
      sleep(4916)
      local sites = 0
      for x = 84, 94 do
         for y = 114, 122 do
            local f = map:get_field(x, y)
            if f.immovable ~= nil and f.immovable.descr.type_name == "militarysite" and f.immovable.fields[1] == f and f.owner == p1 then
               sites = sites + 1
               if sites >= 3 then
                  set_objective_done(o)
                  campaign_message_box(secured_block)
                  return
               end
            end
         end
      end
   end
end

function check_objective_uplands()
   local o = add_campaign_objective(obj_uplands)
   while true do
      sleep(4471)
      local nr = 0
      for _,descr in pairs(p1.tribe.buildings) do
         for _,b in pairs(p1:get_buildings(descr.name)) do
            if b.fields[1].height > 40 then
               nr = nr + 1
               if nr >= 15 then
                  set_objective_done(o)
                  campaign_message_box(secured_uplands)
                  return
               end
            end
         end
      end
   end
end

function mission_thread()
   include "map:scripting/init_p2.lua"
   run(check_fish)
   include "map:scripting/init_p1.lua"
   -- First prepare for scripted vision in the latter
   local first_field = map:get_field(94, 145)
   local reveal_fields = {}
   local owned_fields = {}
   local land_fields = {}
   for x=0,121 do
      for y=17,240 do
         local f = map:get_field(x, y)
         p2:reveal_fields({f})
         if p1:sees_field(f) then
            table.insert(owned_fields, f)
         elseif not f:has_caps("swimmable") then
            table.insert(land_fields, f)
         else
            table.insert(reveal_fields, f)
         end
         p1:hide_fields({f}, "permanent")
      end
   end
   local reveal = {}
   for i=1,6 do
      reveal[i] = {}
   end
   for id, f in ipairs(reveal_fields) do
      table.insert(reveal[(id % 6) + 1], f)
   end
   reveal_fields = nil

   -- start thread
   campaign_message_box(intro_1)
   campaign_message_box(intro_2)
   campaign_message_box(intro_3)

   scroll_to_field(first_field)
   p1:place_ship(first_field)
   reveal_concentric(p1, first_field, 9)
   first_field = nil

   campaign_message_box(intro_4)
   sleep(1000)
   campaign_message_box(intro_5)
   reveal_randomly(p1, owned_fields, 1000, false)
   owned_fields = nil

   campaign_message_box(intro_6)
   campaign_message_box(intro_7)

   scroll_to_field(map.player_slots[2].starting_field)
   reveal_concentric(p1, map.player_slots[2].starting_field, 11)
   sleep(1500)
   scroll_to_field(map.player_slots[1].starting_field)

   campaign_message_box(intro_8)
   campaign_message_box(intro_9)
   campaign_message_box(intro_10)
   local o = add_campaign_objective(obj_fight)

   run(function()
      reveal_randomly(p1, land_fields, 500, false)
      land_fields = nil
   end)

   campaign_message_box(trade_1)

   for i,f in ipairs({
      map:get_field(86, 132),
      map:get_field(82, 124),
      map:get_field(79, 116),
      map:get_field(75, 107),
      map:get_field(70, 99),
      map:get_field(65, 91),
      map:get_field(60, 83),
      map:get_field(55, 75),
      map:get_field(44, 73),
      map:get_field(33, 72),
      map:get_field(37, 80),
      map:get_field(42, 88),
      map:get_field(48, 94),
      map:get_field(54, 102),
      map:get_field(59, 110),
      map:get_field(65, 117),
      map:get_field(69, 128),
      map:get_field(72, 139),
      map:get_field(75, 150),
      map:get_field(79, 160),
      map:get_field(84, 168),
      map:get_field(89, 177),
      map:get_field(89, 160),
   }) do
      sleep(75)
      scroll_to_field(f)
   end
   sleep(100)
   scroll_to_field(map.player_slots[1].starting_field)

   campaign_message_box(trade_2)

   local function run_reveal(array)
      reveal_randomly(p1, array, 1000, false)
   end
   for i=1,6 do
      run(run_reveal, reveal[i])
   end
   reveal = nil

   sleep(1000)
   campaign_message_box(trade_3)
   add_campaign_objective(obj_trade)
   run(check_trade)

   sleep(1000)
   scroll_to_field(map:get_field(78, 141))
   sleep(500)
   campaign_message_box(getting_started_1)
   run(check_objective_uplands)
   sleep(1000)
   scroll_to_field(map:get_field(86, 116))
   sleep(500)
   campaign_message_box(getting_started_2)
   run(check_objective_block)

   sleep(1000)
   scroll_to_field(map:get_field(90, 185))
   sleep(500)
   campaign_message_box(getting_started_3)
   run(check_objective_wood)

   sleep(1000)
   scroll_to_field(map.player_slots[1].starting_field)
   sleep(1000)
   campaign_message_box(getting_started_4)

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
   set_objective_done(o)
   sleep(1500)
   campaign_message_box(victory_1)
   campaign_message_box(victory_2)
   campaign_message_box(victory_3)
   campaign_message_box(victory_4)
   campaign_message_box(victory_5)
   campaign_message_box(victory_6)
   campaign_message_box(victory_7)

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
