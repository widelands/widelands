-- ================
-- Mission thread
-- ================

local function wait_for_quarry_road_connection(field, cs, objective)
   -- Wait till the construction site is connected to the headquarters
   sleep(10 * wl.Game().desired_speed)
   while not field.immovable or field.brn.immovable.debug_worker_economy ~= sf.brn.immovable.debug_worker_economy do
      if not field.immovable then
         campaign_message_box(quarry_illegally_destroyed)
         scroll_to_field(field)
         mouse_to_field(field)

         cs = nil
         immovable_is_legal = function(i)
            -- only allow quarry and flag at this position because the road building below relies on this
            if (i.fields[1] == field) or (i.fields[1] == field.brn) then
               cs = allow_constructionsite(i, {"barbarians_quarry"})
               return cs
            elseif(i.descr.type_name == "flag") or (i.descr.type_name == "road") then
               register_immovable_as_allowed(i)
               return true
            else return false end
         end

         -- Wait for a new constructionsite to be placed
         while not cs do sleep(200) end
         register_immovable_as_allowed(cs)
      else
         campaign_message_box(quarry_not_connected)
      end
      sleep(60*1000)
   end
   set_objective_done(objective, 0)
   register_immovable_as_allowed(cs)
end

function starting_infos()
   -- Let's always start like the default options
   -- (except for automatic road building mode, which is handled by the
   --  scripts)
   wl.ui.MapView().buildhelp = false
   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false

   -- So that the player cannot build anything here
   map:place_immovable("debris00", second_quarry_field)
   reveal_concentric(plr, sf, 13, true, 80)
   sleep(1000)

   -- Welcome and teach objectives
   campaign_message_box(initial_message_01, 200)
   objective_to_explain_objectives = add_campaign_objective(obj_initial_close_objectives_window)

   wl.ui.MapView().buttons.objectives:click()
   while not wl.ui.MapView().windows.objectives do sleep(100) end
   while wl.ui.MapView().windows.objectives do sleep(100) end
   -- TODO(Nordfriese): Re-add training wheels code after v1.0
   -- plr:mark_training_wheel_as_solved("objectives")
   set_objective_done(objective_to_explain_objectives, 500)

   -- Teach building spaces
   wl.ui.MapView().buildhelp = false
   campaign_message_box(initial_message_02, 200)
   show_item_from_dropdown("dropdown_menu_showhide", 1)
   local o = campaign_message_with_objective(initial_message_03, obj_initial_toggle_building_spaces)

   -- Wait for buildhelp to come on
   while not wl.ui.MapView().buildhelp do
      sleep(200)
   end
   set_objective_done(o, 500)
   wl.ui.MapView():abort_road_building()
   -- TODO(Nordfriese): Re-add training wheels code after v1.0
   -- plr:mark_training_wheel_as_solved("building_spaces")

   build_lumberjack()
end

function build_lumberjack()
   sleep(100)

   -- We take control, everything that we build is legal
   immovable_is_legal = function(i) return true end

   campaign_message_box(lumberjack_message_01)

   local blocker = UserInputDisabler:new()
   close_windows()

   scroll_to_field(first_lumberjack_field)
   mouse_to_field(first_lumberjack_field)
   sleep(500)
   campaign_message_box(lumberjack_message_02)
   sleep(500)

   click_on_field(first_lumberjack_field)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.small)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.barbarians_lumberjacks_hut)

   sleep(500)

   if wl.ui.MapView().is_building_road then
      campaign_message_box(lumberjack_message_03a)
   else
      enter_road_building_mode(first_lumberjack_field.brn.immovable)
      campaign_message_box(lumberjack_message_03b)
   end
   sleep(500)

   click_on_field(sf.brn)

   campaign_message_box(lumberjack_message_04)

   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = map:get_field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   immovable_is_legal = function(i) return false end

   blocker:lift_blocks()

   sleep(20000)

   if not (f.immovable and f.immovable.descr.type_name == "flag") then
      -- only show this if the user has not already built a flag
      local o = campaign_message_with_objective(lumberjack_message_05, obj_lumberjack_place_flag)

      local blocker = UserInputDisabler:new()
      close_windows()

      local f = map:get_field(14,11)
      scroll_to_field(f)
      mouse_to_field(f)

      blocker:lift_blocks()

      -- Wait for flag
      while not (f.immovable and f.immovable.descr.type_name == "flag") do sleep(300) end
      set_objective_done(o, 16 * 1000)
   else
      -- if the flag is already built, show the player a different message box
      campaign_message_box(lumberjack_message_06, 3 * 1000)
   end

   -- TODO(Nordfriese): Re-add training wheels code after v1.0
   -- plr:mark_training_wheel_as_solved("flags")

   local o = campaign_message_with_objective(lumberjack_message_07a, obj_lumberjack_progress)
   scroll_to_field(first_lumberjack_field)
   mouse_to_field(first_lumberjack_field)

   while not wl.ui.MapView().windows.building_window do sleep(100) end
   -- demonstrate work area button
   blocker = UserInputDisabler:new()
   sleep(1000)
   campaign_message_box(lumberjack_message_07b, 1000)
   click_on_panel(wl.ui.MapView().windows.building_window.buttons.workarea)
   blocker:lift_blocks()

   while wl.ui.MapView().windows.building_window do sleep(100) end
   set_objective_done(o)
   sleep(3000)

   campaign_message_box(lumberjack_message_08)
   show_item_from_dropdown("dropdown_menu_gamespeed", 1)

   sleep(20*1000) -- let the player experiment a bit with the window

   while #plr:get_buildings("barbarians_lumberjacks_hut") < 1 do sleep(300) end

   campaign_message_box(lumberjack_message_09)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   local o = campaign_message_with_objective(tell_about_move, obj_moving)

   function _wait_for_move()
      local center_map_pixel = wl.ui.MapView().center_map_pixel
      while center_map_pixel.x == wl.ui.MapView().center_map_pixel.x and
            center_map_pixel.y == wl.ui.MapView().center_map_pixel.y do
         sleep(300)
      end
   end

   _wait_for_move()
   set_objective_done(o)


   -- Teach the minimap
   campaign_message_box(tell_about_minimap_1)

   -- Open the minimap
   show_item_from_dropdown("dropdown_menu_mapview", 1)
   select_item_from_dropdown("dropdown_menu_mapview", 1)
   o = campaign_message_with_objective(tell_about_minimap_2, obj_moving_minimap)

   -- Wait until the minimap has been opened and closed again
   while not wl.ui.MapView().windows.minimap do sleep(100) end
   while wl.ui.MapView().windows.minimap do sleep(100) end

   set_objective_done(o, 500)

   campaign_message_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end

function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   local o = campaign_message_with_objective(order_quarry_recap_how_to_build, obj_build_a_quarry)

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

   set_objective_done(o, 0)
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

   sleep(wl.Game().desired_speed) -- give the game some time to enter road building mode
   if wl.ui.MapView().is_building_road then
      campaign_message_box(talk_about_roadbuilding_00a)
   else
      -- show the user how to enter road building mode manually
      campaign_message_box(talk_about_roadbuilding_00b)
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

   campaign_message_box(talk_about_roadbuilding_01)
   -- Showoff direct roadbuilding
   click_on_field(first_quarry_field.brn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   click_on_field(sf.brn)

   sleep(3000)

   _rip_road()

   blocker:lift_blocks()

   local o = campaign_message_with_objective(talk_about_roadbuilding_02, obj_build_road_to_quarry)

   -- The player is allowed to build roads and flags at will
   immovable_is_legal = function(i)
      if (i.descr.type_name == "flag") or (i.descr.type_name == "road") then
         register_immovable_as_allowed(i)
         return true
      else return false end
   end

   wait_for_quarry_road_connection(first_quarry_field, cs, o)

   second_quarry()
   -- TODO(Nordfriese): Re-add training wheels code after v1.0
   -- plr:mark_training_wheel_as_solved("logs")
   -- plr:mark_training_wheel_as_solved("rocks")

   -- Interludium: talk about census and statistics
   census_and_statistics()

   while #plr:get_buildings("barbarians_quarry") < 2 do sleep(1400) end

   messages()
end

function second_quarry()
   sleep(2000)

   local o = campaign_message_with_objective(build_second_quarry, obj_build_the_second_quarry)
   -- Remove this immovable (debris)
   second_quarry_field.immovable:remove()
   scroll_to_field(first_quarry_field)
   mouse_to_field(second_quarry_field)

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
   wait_for_quarry_road_connection(second_quarry_field, cs, o)
end

function census_and_statistics()
   sleep(10000)

   local blocker = UserInputDisabler:new()
   close_windows()

   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false

   wl.ui.MapView():abort_road_building()

   campaign_message_box(census_and_statistics_00)

   show_item_from_dropdown("dropdown_menu_showhide", 2)
   select_item_from_dropdown("dropdown_menu_showhide", 2)
   sleep(200)

   blocker:lift_blocks()

   local o = campaign_message_with_objective(census_and_statistics_01, obj_show_statistics)

   -- Wait for statistics to come on
   while not wl.ui.MapView().statistics do sleep(200) end
   set_objective_done(o, 5 * wl.Game().desired_speed)

   if (#plr:get_buildings("barbarians_quarry") < 2) then
      campaign_message_box(census_and_statistics_02, 200)
   end
end

function messages()
   -- Teach the player about receiving messages
   sleep(10)
   local old_gamespeed = wl.Game().desired_speed
   wl.Game().desired_speed = 1000

   send_to_inbox(plr, teaching_about_messages.title, teaching_about_messages.body, teaching_about_messages, {heading = teaching_about_messages.heading})
   local o = add_campaign_objective(obj_archive_all_messages)

   while #plr.inbox > 0 do sleep(200) end
   set_objective_done(o, 500)

   local o = campaign_message_with_objective(closing_msg_window_00, obj_close_message_window)

   -- Wait for messages window to close
   while wl.ui.MapView().windows.messages do sleep(300) end
   set_objective_done(o, 300)

   campaign_message_box(closing_msg_window_01, 800)

   if (wl.Game().desired_speed == 1000) then wl.Game().desired_speed = old_gamespeed end

   destroy_quarries()
end

function destroy_quarries()
   sleep(200)
   -- Remove all rocks
   remove_all_rocks(first_quarry_field:region(6))

   function count_quarry_messages()
      local count = 0
      for i, msg in ipairs(plr.messages) do
         if (msg.field == first_quarry_field or msg.field == second_quarry_field) then
            count = count + 1
         end
      end
      return count
   end

   -- Wait for messages to arrive
   while count_quarry_messages() < 2 do sleep(300) end

   local o = campaign_message_with_objective(destroy_quarries_message, obj_destroy_quarries)

   while #plr:get_buildings("barbarians_quarry") > 0 do sleep(200) end
   set_objective_done(o)

   expansion()
end

function expansion()
   -- Teach about expanding the territory
   sleep(10)

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true

   local o = campaign_message_with_objective(introduce_expansion, obj_expand_territory)

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

   set_objective_done(o, 2 * wl.Game().desired_speed)
   campaign_message_box(military_building_finished)

   conclusion()
end

function conclusion()
   sleep(5 * wl.Game().desired_speed) -- to give the player time to see his expanded area

   -- Conclude the tutorial with final words and information
   -- on how to quit
   campaign_message_box(conclude_tutorial)
end

run(bad_boy_sentry)
run(starting_infos)
