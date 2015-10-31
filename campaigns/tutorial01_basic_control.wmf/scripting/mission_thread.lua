-- ================
-- Mission thread
-- ================

function starting_infos()
   map:place_immovable("debris00",second_quarry_field, "world")
   -- so that the player cannot build anything here

   sleep(1000)

   message_box_objective(plr, initial_message_01)
   sleep(500)

   local o = message_box_objective(plr, initial_message_02)

   -- Wait for buildhelp to come on
   while not wl.ui.MapView().buildhelp do
      sleep(200)
   end
   o.done = true
   wl.ui.MapView():abort_road_building()

   sleep(500)

   build_lumberjack()
end

function build_lumberjack()
   sleep(100)

   -- We take control, everything that we build is legal
   immovable_is_legal = function(i) return true end

   message_box_objective(plr, lumberjack_message_01)

   local blocker = UserInputDisabler:new()
   close_windows()

   scroll_smoothly_to(first_lumberjack_field)
   mouse_smoothly_to(first_lumberjack_field)
   sleep(500)
   message_box_objective(plr, lumberjack_message_02)
   sleep(500)

   click_on_field(first_lumberjack_field)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.small)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.barbarians_lumberjacks_hut)

   sleep(500)

   if wl.ui.MapView().is_building_road then
      message_box_objective(plr, lumberjack_message_03a)
   else
      enter_road_building_mode(first_lumberjack_field.brn.immovable)
      message_box_objective(plr, lumberjack_message_03b)
   end
   sleep(500)

   click_on_field(sf.brn)

   message_box_objective(plr, lumberjack_message_04)

   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = map:get_field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   immovable_is_legal = function(i) return false end

   blocker:lift_blocks()

   sleep(15000)

   if not (f.immovable and f.immovable.descr.type_name == "flag") then
      -- only show this if the user has not already built a flag
      local o = message_box_objective(plr, lumberjack_message_05)

      local blocker = UserInputDisabler:new()
      close_windows()

      local f = map:get_field(14,11)
      scroll_smoothly_to(f)
      mouse_smoothly_to(f)

      blocker:lift_blocks()

      -- Wait for flag
      while not (f.immovable and f.immovable.descr.type_name == "flag") do sleep(300) end
      o.done = true

      sleep(300)

      message_box_objective(plr, lumberjack_message_06)
   else
      -- if the flag is already built, show the player a different message box
      message_box_objective(plr, flag_built)
   end

   sleep(30*1000) -- let the player experiment a bit with the speed
   message_box_objective(plr, construction_site_window)

   while #plr:get_buildings("barbarians_lumberjacks_hut") < 1 do sleep(300) end

   message_box_objective(plr, lumberjack_message_07)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   local o = message_box_objective(plr, inform_about_rocks)

   function _wait_for_move()
      local cx = wl.ui.MapView().viewpoint_x
      local cy = wl.ui.MapView().viewpoint_y
      while cx == wl.ui.MapView().viewpoint_x and
            cy == wl.ui.MapView().viewpoint_y do
         sleep(300)
      end
   end

   _wait_for_move()
   o.done = true
   sleep(3000) -- Give the player a chance to try this some more

   o = message_box_objective(plr, tell_about_right_drag_move)

   _wait_for_move()
   o.done = true
   sleep(3000) -- Give the player a chance to try this some more

   o = message_box_objective(plr, tell_about_minimap)

   -- Wait until the minimap has been opened and closed again
   while not wl.ui.MapView().windows.minimap do sleep(100) end
   while wl.ui.MapView().windows.minimap do sleep(100) end

   o.done = true
   sleep(500)

   message_box_objective(plr, congratulate_and_on_to_quarry)

   build_a_quarry()
end

function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   local o = message_box_objective(plr, order_quarry_recap_how_to_build)

   local cs = nil
   immovable_is_legal = function(i)
      -- only allow quarry and flag at this position because the road building below relies on this
      if (i.fields[1] == first_quarry_field) or (i.fields[1] == first_quarry_field.brn) then
         cs = allow_constructionsite(i, {"barbarians_quarry"})
         return cs
      else return false end
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(200) end

   o.done = true
   register_immovable_as_allowed(cs)

   local function _rip_road()
      for idx,f in ipairs(first_quarry_field.brn:region(2)) do
         if f.immovable and f.immovable.descr.type_name == "road" then
            click_on_field(f)
            click_on_panel(wl.ui.MapView().windows.
               field_action.buttons.destroy_road, 300)
            sleep(200)
            return
         end
      end
   end

   local blocker = UserInputDisabler:new()
   close_windows()

   immovable_is_legal = function() return true end

   sleep(3000) -- give the game some time to enter road building mode
   if wl.ui.MapView().is_building_road then
      message_box_objective(plr, talk_about_roadbuilding_00a)
   else
      -- show the user how to enter road building mode manually
      message_box_objective(plr, talk_about_roadbuilding_00b)
      click_on_field(first_quarry_field.brn)
      click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   end

   -- Showoff one-by-one roadbuilding
   click_on_field(map:get_field(9,12))
   click_on_field(map:get_field(10,12))
   click_on_field(map:get_field(11,12))
   click_on_field(map:get_field(12,12))
   click_on_field(map:get_field(12,11))

   sleep(3000)

   _rip_road()

   message_box_objective(plr, talk_about_roadbuilding_01)
   -- Showoff direct roadbuilding
   click_on_field(first_quarry_field.brn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   click_on_field(sf.brn)

   sleep(3000)

   _rip_road()

   blocker:lift_blocks()

   local o = message_box_objective(plr, talk_about_roadbuilding_02)

   -- The player is allowed to build roads and flags at will
   immovable_is_legal = function(i)
      if (i.descr.type_name == "flag") or (i.descr.type_name == "road") then
         register_immovable_as_allowed(i)
         return true
      else return false end
   end

   -- Wait till the construction site is connected to the headquarters
   sleep(20*1000)
   while first_quarry_field.brn.immovable.debug_economy ~= sf.brn.immovable.debug_economy do
      message_box_objective(plr,quarry_not_connected)
      sleep(60*1000)
      if not first_quarry_field.immovable then message_box_objective(plr,quarry_illegally_destroyed) return end
   end

   second_quarry()

   -- Interludium: talk about census and statistics
   census_and_statistics()

   while #plr:get_buildings("barbarians_quarry") < 2 do sleep(1400) end
   o.done = true

   messages()
end

function second_quarry()
   sleep(2000)

   local o = message_box_objective(plr, build_second_quarry)
   second_quarry_field.immovable:remove()
   -- remove this immovable (debris)

   local cs = nil
   immovable_is_legal = function(i)
      if (i.fields[1] == second_quarry_field) or (i.fields[1] == second_quarry_field.brn) then
         cs = allow_constructionsite(i, {"barbarians_quarry"})
         return cs
      elseif(i.descr.type_name == "flag") or (i.descr.type_name == "road") then
         register_immovable_as_allowed(i)
         return true
      else return false end
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(200) end

   sleep(60*1000)
   while second_quarry_field.brn.immovable.debug_economy ~= sf.brn.immovable.debug_economy do
      message_box_objective(plr,quarry_not_connected)
      sleep(60*1000)
      if not second_quarry_field.immovable then message_box_objective(plr,quarry_illegally_destroyed) return end
   end

   o.done = true
   register_immovable_as_allowed(cs)
end


function census_and_statistics()
   sleep(15000)

   local blocker = UserInputDisabler:new()
   close_windows()

   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false

   wl.ui.MapView():abort_road_building()

   message_box_objective(plr, census_and_statistics_00)

   click_on_field(first_quarry_field.bln)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.census)
   sleep(300)
   click_on_field(first_quarry_field.brn)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.statistics)

   message_box_objective(plr, census_and_statistics_01)

   blocker:lift_blocks()
end

function messages()
   -- Teach the player about receiving messages
   sleep(10)

   send_message(plr, teaching_about_messages.title, teaching_about_messages.body, teaching_about_messages)
   local o = add_campaign_objective(teaching_about_messages)

   while #plr.inbox > 0 do sleep(200) end
   o.done = true

   sleep(500)

   local o = message_box_objective(plr, closing_msg_window_00)

   -- Wait for messages window to close
   while wl.ui.MapView().windows.messages do sleep(300) end
   o.done = true

   message_box_objective(plr, closing_msg_window_01)

   sleep(800)

   destroy_quarries()
end

function destroy_quarries()
   sleep(200)
   -- Remove all rocks
   remove_all_rocks(first_quarry_field:region(6))

   -- Wait for messages to arrive
   while #plr.inbox < 2 do sleep(300) end

   local o = message_box_objective(plr, destroy_quarries_message)

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true

   while #plr:get_buildings("barbarians_quarry") > 0 do sleep(200) end
   o.done = true

   sleep(3000)

   expansion()
end

function expansion()
   -- Teach about expanding the territory
   sleep(10)

   local o = message_box_objective(plr, introduce_expansion)

   -- wait until there are soldiers inside so that the player sees the expansion
   local soldier_inside = false
   while not soldier_inside do
      local military_buildings = array_combine(
         plr:get_buildings("barbarians_sentry"),
         plr:get_buildings("barbarians_tower"),
         plr:get_buildings("barbarians_barrier"),
         plr:get_buildings("barbarians_fortress"),
         plr:get_buildings("barbarians_citadel"))
      for i = 1,#military_buildings do
         for k,v in pairs(military_buildings[i]:get_soldiers("all")) do
            soldier_inside = true
            break
         end
      end
      sleep(500)
   end

   o.done = true
   sleep(4000)
   message_box_objective(plr, military_building_finished)

   conclusion()
end

function conclusion()
   sleep(10000) -- to give the player time to see his expanded area

   -- Conclude the tutorial with final words and information
   -- on how to quit
   message_box_objective(plr, conclude_tutorial)

end

run(bad_boy_sentry)
run(starting_infos)