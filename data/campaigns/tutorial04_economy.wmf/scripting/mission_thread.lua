-- ===============
-- Mission thread
-- ===============

function introduction()
   sleep(1000)
   message_box_objective(plr, intro1)
   message_box_objective(plr, intro2)

   burn_tavern_down()
end

function burn_tavern_down()
   sleep(500)
   scroll_smoothly_to(tavern_field)
   sleep(500)
   tavern_field.immovable:destroy()
   sleep(500)
   message_box_objective(plr, tavern_burnt_down)
   sleep(500)
   local o = message_box_objective(plr, building_stat)
   while not mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, wl.Game().real_speed)

   o = message_box_objective(plr,explain_building_stat)
   -- We cannot create several objectives with the same name. Therefore, we create o2 here once and change its visibility
   local o2 = add_campaign_objective(reopen_building_stat_obj)
   o2.visible = false
   local medium_tab_active = false
   while not medium_tab_active do
      if not mv.windows.building_statistics then
         o2.visible = true
         message_box_objective(plr, reopen_building_stat)
         while not mv.windows.building_statistics do sleep(200) end
         o2.visible = false
      end
      if mv.windows.building_statistics.tabs["building_stats_medium"].active then medium_tab_active = true end
      sleep(200)
   end
   while mv.windows.building_statistics do sleep(100) end
   set_objective_done(o, 0)

   o = message_box_objective(plr, inventory1)
   while not mv.windows.stock_menu do sleep(200) end
   set_objective_done(o, 0)

   o = message_box_objective(plr, inventory2)
   -- We cannot create several objectives with the same name. Therefore, we create o2 here once and change its visibility
   o2 = add_campaign_objective(reopen_stock_menu_obj)
   o2.visible = false
   while not mv.windows.stock_menu.tabs["wares_in_warehouses"].active do
      if not mv.windows.stock_menu then
         o2.visible = true
         message_box_objective(plr, reopen_stock_menu)
         while not mv.windows.stock_menu do sleep(200) end
         o2.visible = false
      end
      sleep(200)
   end
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
   while not mv.windows.ware_statistics.tabs["economy_health"].active do
      if not mv.windows.ware_statistics then
         o2.visible = true
         message_box_objective(plr, reopen_ware_stats1)
         while not mv.windows.ware_statistics do sleep(200) end
         o2.visible = false
      end
      sleep(200)
   end
   set_objective_done(o, 0)

   o = message_box_objective(plr, ware_stats3)
   o2 = add_campaign_objective(reopen_ware_stats2_obj)
   o2.visible = false
   while not mv.windows.ware_statistics.tabs["stock"].active  do
      if not mv.windows.ware_statistics then
         o2.visible = true
         message_box_objective(plr, reopen_ware_stats2)
         while not mv.windows.ware_statistics do sleep(200) end
         o2.visible = false
      end
      sleep(200)
   end
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
