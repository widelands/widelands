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
      "encyclopedia_wares",
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
   o = campaign_message_with_objective(economy_settings3, obj_produce_marble_columns)

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
   conclude()
end

function conclude()
   campaign_message_box(conclusion)
end

run(introduction)
