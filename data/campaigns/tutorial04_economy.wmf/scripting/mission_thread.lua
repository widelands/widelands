-- ===============
-- Mission thread
-- ===============

function introduction()
   init_player()
   fields = get_sees_fields(plr)
   reveal_randomly(plr, fields, 2000)
   remaining_roads()

   sleep(1000)

   campaign_message_box(intro1)
   campaign_message_box(intro2)

   burn_tavern_down()
end

function wait_for_window_and_tab_or_complain(
   window_name,
   tab_name,
   complain_msg,
   objective)

   local obj_open_window = add_campaign_objective(objective)
   obj_open_window.visible = false

   while true do
      -- This waits for the window to be opened.
      if not mv.windows[window_name] then
         obj_open_window.visible = true
         campaign_message_box(complain_msg)
         while not mv.windows[window_name] do sleep(200) end
         obj_open_window.visible = false
      end

      -- But it might be closed at any point in time. If it is open and the
      -- correct tab is active, we terminate the loop.
      if mv.windows[window_name] and
         mv.windows[window_name].tabs[tab_name].active
      then
         break
      end
      sleep(200)
   end
   set_objective_done(obj_open_window)
end

function encyclopedia_tutorial()
   sleep(100*1000)
   local o = campaign_message_with_objective(ware_encyclopedia, obj_open_encyclopedia) -- where to get help
   while not mv.windows.encyclopedia do sleep(200) end
   set_objective_done(o, wl.Game().real_speed)

   o = campaign_message_with_objective(explain_encyclopedia, obj_lookup_wares) -- what information is available
   wait_for_window_and_tab_or_complain(
      "encyclopedia",
      "tab_encyclopedia_wares",
      reopen_encyclopedia, obj_reopen_encyclopedia
   )
   while mv.windows.encyclopedia do sleep(200) end
   set_objective_done(o, wl.Game().real_speed)
end

function burn_tavern_down()
   sleep(500)
   scroll_to_field(tavern_field)
   sleep(1000)
   tavern_field.immovable:destroy()
   sleep(1000)
   campaign_message_box(tavern_burnt_down)
   sleep(500)
   local o = campaign_message_with_objective(building_stats, obj_open_building_stats)
   wl.ui.MapView().dropdowns["dropdown_menu_statistics"]:open()

   while not mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, wl.Game().real_speed)

   o = campaign_message_with_objective(explain_building_stats, obj_check_taverns)
   wait_for_window_and_tab_or_complain(
      "building_statistics",
      "building_stats_medium",
      reopen_building_stats, obj_reopen_building_stats
   )
   while mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, 0)

   o = campaign_message_with_objective(inventory1, obj_open_inventory)
   while not mv.windows.stock_menu do sleep(200) end
   set_objective_done(o, wl.Game().real_speed)

   o = campaign_message_with_objective(inventory2, obj_switch_stock_tab)
   wait_for_window_and_tab_or_complain(
      "stock_menu",
      "total_wares",
      reopen_stock_menu, obj_reopen_stock_menu
   )
   wait_for_window_and_tab_or_complain(
      "stock_menu",
      "wares_in_warehouses",
      reopen_stock_menu2, obj_reopen_stock_menu2
   )
   set_objective_done(o, 0)
   campaign_message_box(inventory3)

   sleep(2000)
   o = campaign_message_with_objective(build_taverns, obj_build_taverns)

   encyclopedia_tutorial()

   while #plr:get_buildings("empire_tavern") < 2 do sleep(500) end
   set_objective_done(o, 0)

   plan_the_future()
end

function plan_the_future()
   campaign_message_box(building_priority_settings)
   sleep(30*1000) -- give the user time to try it out

   local o = campaign_message_with_objective(ware_stats1, obj_open_ware_stats)
   while not mv.windows.ware_statistics do sleep(200) end
   set_objective_done(o, 0)

   o = campaign_message_with_objective(ware_stats2, obj_switch_ware_stats_tab_to_third)
   wait_for_window_and_tab_or_complain(
      "ware_statistics",
      "economy_health",
      reopen_ware_stats1, obj_reopen_ware_stats1
   )
   set_objective_done(o, 0)

   o = campaign_message_with_objective(ware_stats3, obj_switch_ware_stats_tab_to_fourth)
   wait_for_window_and_tab_or_complain(
      "ware_statistics",
      "stock",
      reopen_ware_stats2, obj_reopen_ware_stats2
   )
   set_objective_done(o, 0)

   o = campaign_message_with_objective(ware_stats4, obj_close_ware_stats)
   while mv.windows.ware_statistics do sleep(500) end
   set_objective_done(o)

   o = campaign_message_with_objective(economy_settings1, obj_open_economy_settings)
   while not mv.windows.economy_options do sleep(200) end
   set_objective_done(o, 0)
   campaign_message_box(economy_settings2)
   sleep(200)
   campaign_message_box(economy_settings3)
   sleep(200)
   campaign_message_box(economy_settings4)
   o = campaign_message_with_objective(economy_settings5, obj_produce_marble_columns)

   while sf.brn.immovable.ware_economy:target_quantity("marble_column") ~= 20 do
      sleep(200)
   end
   -- wait that the player has really changed the target quantity
   set_objective_done(o)

   -- new objective all has to be transported to the front
   o = campaign_message_with_objective(warehouse_preference_settings, obj_bring_marble_columns_to_front)

   local enough_wares = false
   while not enough_wares do
      if (warehouse_field.immovable and
          (warehouse_field.immovable.descr.name == "empire_warehouse")) then
         if warehouse_field.immovable:get_wares("marble_column") >= 20 then
            enough_wares = true
         end
      end
      sleep(500)
   end
   set_objective_done(o)

   -- if the minimum_storage_per_warehouse feature is introduced, use the gold mountain to the northeast for explanation

   trading()
end

function trading()
   prefilled_buildings(plr2,
      { "frisians_headquarters", 39, 89,
         wares = {
            log = 40,
            granite = 50,
            reed = 50,
            brick = 80,
            clay = 30,
            water = 10,
            fish = 10,
            meat = 10,
            fruit = 10,
            barley = 5,
            ration = 20,
            honey = 10,
            smoked_meat = 5,
            smoked_fish = 5,
            mead = 5,
            meal = 2,
            coal = 20,
            iron = 5,
            gold = 4,
            iron_ore = 10,
            bread_frisians = 15,
            honey_bread = 5,
            beer = 5,
            cloth = 5,
            fur = 10,
            fur_garment = 5,
            sword_short = 5,
            hammer = 10,
            fire_tongs = 2,
            bread_paddle = 2,
            kitchen_tools = 2,
            felling_ax = 3,
            needles = 1,
            basket = 2,
            pick = 5,
            shovel = 5,
            scythe = 3,
            hunting_spear = 2,
            fishing_net = 3,
         },
         workers = {
            frisians_carrier = 100,
            frisians_reindeer = 100,
            frisians_farmer = 100,
         }
      },
      { "frisians_sentinel", 51, 85, soldiers = {[{0,0,0,0}] = 1}},
      { "frisians_farm", 53, 85 },
      { "frisians_farm", 42, 87 },
      { "frisians_reindeer_farm", 49, 85 },
      { "frisians_market", 46, 87 },
      { "frisians_quarry", 36, 91 },
      { "frisians_well", 37, 89 },
      { "frisians_well", 40, 87 },
      { "frisians_well", 44, 87 },
      { "frisians_well", 48, 87 }
   )
   connected_road("normal", plr2, map:get_field(37, 92).immovable, "tr,tr|r,r|tr,tr|r,r|r,r|r,r|r,r|tr,tr|r,r|r,r")

   sleep(2000)
   local center_pixel = scroll_to_field(sf2)
   sleep(500)
   reveal_concentric(plr, sf2, 5)
   sleep(3000)

   campaign_message_box(trading_1)

   sleep(500)
   hide_concentric(plr, sf2, 6, false)
   hide_concentric(plr, sf2, 6, false)  -- Hide twice and with a higher radius to ensure the big buildings are hidden completely.
   scroll_to_map_pixel(center_pixel);

   local o = campaign_message_with_objective(trading_2, obj_build_market)

   sleep(50000)
   campaign_message_box(trading_3)

   while #plr:get_buildings("empire_market") == 0 do sleep(500) end
   set_objective_done(o)
   local p1marketpos = plr:get_buildings("empire_market")[1].fields[1]
   local p2market = plr2:get_buildings("frisians_market")[1]
   local p2hq = plr2:get_buildings("frisians_headquarters")[1]

   scroll_to_field(p1marketpos)
   campaign_message_box(trading_4)
   mv:click(p1marketpos)
   sleep(3000)
   campaign_message_box(trading_5)
   campaign_message_box(trading_6)
   campaign_message_box(trading_7)

   local o = campaign_message_with_objective(trading_8, obj_propose_trade)

   local trade_accepted = false
   while not trade_accepted do
      sleep(1000)
      for i,trade in ipairs(wl.Game().trades) do
         if trade.state == "proposed" then
            local send = 0
            local receive = 0
            for ware,amount in pairs(trade.items_to_send) do
               send = send + amount
            end
            for ware,amount in pairs(trade.items_to_receive) do
               receive = receive + amount
            end
            if receive > send then
               plr2:reject_trade(trade.trade_id)
               campaign_message_box(trading_rejected)
            else
               p2market:accept_trade(trade.trade_id)
               trade_accepted = true

               local num_batches = trade.num_batches
               if num_batches < 0 then  -- infinite
                  num_batches = 106  -- kind of high
               else
                  num_batches = num_batches + 3  -- some extra
               end
               for ware,amount in pairs(trade.items_to_receive) do
                  p2hq:set_wares(ware, math.max(p2hq:get_wares(ware), amount * (num_batches)))
               end
            end
         end
      end
   end

   set_objective_done(o)
   mv:click(p1marketpos)
   campaign_message_box(trading_9)

   sleep(15000)
   local new_trade_id = p2market:propose_trade(plr, math.random(3, 20), {
      granite = 1,
      water = 4,
   }, {
      iron_ore = 1,
      gold_ore = 1,
   })

   local o = campaign_message_with_objective(trading_10, obj_open_diplomacy)
   while not mv.windows.diplomacy do sleep(200) end
   set_objective_done(o)

   local o = campaign_message_with_objective(trading_11, obj_decide_trade_offer)
   while true do
      sleep(1000)
      local t = wl.Game():get_trade(new_trade_id)
      if t == nil or t.state ~= "proposed" then
         break
      end
   end
   set_objective_done(o)
   campaign_message_box(trading_12)

   conclude()
end

function conclude()
   campaign_message_box(conclusion)
end

run(introduction)
