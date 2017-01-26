-- ===============
-- Mission thread
-- ===============

function introduction()
   sleep(1000)
   message_box_objective(plr, intro1)
   message_box_objective(plr, intro2)

   burn_tavern_down()
end

function wait_for_window_and_tab_or_complain(
   window_name,
   tab_name,
   objective, complain_msg
)
   while true do
      -- This waits for the window to be opened.
      if not mv.windows[window_name] then
         objective.visible = true
         message_box_objective(plr, complain_msg)
         while not mv.windows[window_name] do sleep(200) end
         objective.visible = false
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
end

function burn_tavern_down()
   sleep(500)
   scroll_to_field(tavern_field)
   sleep(1000)
   tavern_field.immovable:destroy()
   sleep(1000)
   message_box_objective(plr, tavern_burnt_down)
   sleep(500)
   local o = message_box_objective(plr, building_stat)
   while not mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, wl.Game().real_speed)

   o = message_box_objective(plr,explain_building_stat)
   -- We cannot create several objectives with the same name. Therefore, we create o2 here once and change its visibility
   local o2 = add_campaign_objective(reopen_building_stat_obj)
   o2.visible = false
   wait_for_window_and_tab_or_complain(
      "building_statistics",
      "building_stats_medium",
      o2, reopen_building_stat
   )
   while mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, 0)

   o = message_box_objective(plr, inventory1)
   while not mv.windows.stock_menu do sleep(200) end
   set_objective_done(o, wl.Game().real_speed)

   o = message_box_objective(plr, inventory2)
   -- We cannot create several objectives with the same name. Therefore, we
   -- create o2 here once and change its visibility
   o2 = add_campaign_objective(reopen_stock_menu_obj)
   o2.visible = false

   wait_for_window_and_tab_or_complain(
      "stock_menu",
      "wares_in_warehouses",
      o2, reopen_stock_menu
   )
   set_objective_done(o, 0)
   message_box_objective(plr, inventory3)

   sleep(2000)
   o = message_box_objective(plr, build_taverns)

   sleep(100*1000)
   message_box_objective(plr, ware_encyclopedia) -- a small insert

   while #plr:get_buildings("empire_tavern") < 2 do sleep(500) end
   set_objective_done(o, 0)

   plan_the_future()
end

function plan_the_future()
   message_box_objective(plr, building_priority_settings)
   sleep(30*1000) -- give the user time to try it out

   local o = message_box_objective(plr, ware_stats1)
   while not mv.windows.ware_statistics do sleep(200) end
   set_objective_done(o, 0)

   o = message_box_objective(plr, ware_stats2)
   local o2 = add_campaign_objective(reopen_ware_stats1_obj)
   o2.visible = false

   wait_for_window_and_tab_or_complain(
      "ware_statistics",
      "economy_health",
      o2, reopen_ware_stats1
   )
   set_objective_done(o, 0)

   o = message_box_objective(plr, ware_stats3)
   o2 = add_campaign_objective(reopen_ware_stats2_obj)
   o2.visible = false

   wait_for_window_and_tab_or_complain(
      "ware_statistics",
      "stock",
      o2, reopen_ware_stats2
   )
   set_objective_done(o, 0)

   o = message_box_objective(plr, ware_stats4)
   while mv.windows.ware_statistics do sleep(500) end
   set_objective_done(o)

   o = message_box_objective(plr, economy_settings1)
   while not mv.windows.economy_options do sleep(200) end
   set_objective_done(o, 0)
   message_box_objective(plr, economy_settings2)
   o = message_box_objective(plr, economy_settings3)

   while sf.immovable:get_wares("marble_column") < 12 do sleep(500) end
   -- wait that the player has really changed the target quantity

   o.visible = false
   -- just forget about the old objective, the new one includes the old one
   o = message_box_objective(plr, warehouse_preference_settings)

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
   message_box_objective(plr, conclusion)
end


run(init_player)
run(introduction)
