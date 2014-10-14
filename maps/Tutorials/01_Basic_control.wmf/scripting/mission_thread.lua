-- ================
-- Mission thread
-- ================
function starting_infos()
   map:place_immovable("debris00",second_quarry_field)
   -- so that the player cannot build anything here

   sleep(100)

   msg_box(initial_message_01)
   sleep(500)

   local o = msg_box(initial_message_02)

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

   msg_box(lumberjack_message_01)

   local blocker = UserInputDisabler:new()
   close_windows()

   scroll_smoothly_to(first_lumberjack_field)
   mouse_smoothly_to(first_lumberjack_field)
   sleep(500)
   msg_box(lumberjack_message_02)
   sleep(500)

   click_on_field(first_lumberjack_field)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.small)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.lumberjacks_hut)

   enter_road_building_mode(first_lumberjack_field.brn.immovable)

   sleep(500)
   msg_box(lumberjack_message_03)
   sleep(500)

   click_on_field(map.player_slots[1].starting_field.brn)

   msg_box(lumberjack_message_04)

   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = map:get_field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   immovable_is_legal = function(i) return false end

   blocker:lift_blocks()

   sleep(15000)

   if not (f.immovable and f.immovable.descr.type_name == "flag") then
      -- only show this if the user has not already built a flag
      local o = msg_box(lumberjack_message_05)

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

      msg_box(lumberjack_message_06)
   else
      -- if the flag is already built, show the player a different message box
      msg_box(flag_built)
   end



   sleep(30*1000) -- let the player experiment a bit with the speed
   msg_box(construction_site_window)

   while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end

   msg_box(lumberjack_message_07)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   local o = msg_box(inform_about_rocks)

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

   o = msg_box(tell_about_right_drag_move)

   _wait_for_move()
   o.done = true
   sleep(3000) -- Give the player a chance to try this some more

   o = msg_box(tell_about_minimap)

   -- Wait until the minimap has been opened and closed again
   while not wl.ui.MapView().windows.minimap do sleep(100) end
   while wl.ui.MapView().windows.minimap do sleep(100) end

   o.done = true
   sleep(500)

   msg_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end

function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   local o = msg_box(order_quarry_recap_how_to_build)

   local cs = nil
   immovable_is_legal = function(i)
      -- only allow quarry and flag at this position because the road building below relies on this
      if (i.fields[1] == first_quarry_field) or (i.fields[1] == first_quarry_field.brn) then
         cs = allow_constructionsite(i, {"quarry"})
         return cs
      else return false end
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(200) end

   o.done = true
   register_immovable_as_allowed(cs)

   enter_road_building_mode(cs.fields[1].brn.immovable)

   local function _rip_road()
      for idx,f in ipairs(cs.fields[1].brn:region(2)) do
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

   msg_box(talk_about_roadbuilding_00)
   -- Showoff one-by-one roadbuilding
   click_on_field(map:get_field(9,12))
   click_on_field(map:get_field(10,12))
   click_on_field(map:get_field(11,12))
   click_on_field(map:get_field(12,12))
   click_on_field(map:get_field(12,11))

   sleep(3000)

   _rip_road()

   msg_box(talk_about_roadbuilding_01)
   -- Showoff direct roadbuilding
   click_on_field(cs.fields[1].brn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   click_on_field(map.player_slots[1].starting_field.brn)

   sleep(3000)

   _rip_road()

   blocker:lift_blocks()

   local o = msg_box(talk_about_roadbuilding_02)

   -- The player is allowed to build roads and flags at will
   immovable_is_legal = function(i)
      if (i.descr.type_name == "flag") or (i.descr.type_name == "road") then
         register_immovable_as_allowed(i)
         return true
      else return false end
   end

   -- Give the player some time to build the road
   -- It is not possible to check for the road. See https://bugs.launchpad.net/widelands/+bug/1380286
   sleep(20*1000)

   second_quarry()

   -- Wait a while
   sleep(60*1000)
   -- When the said bug is fixed, check every 30 seconds if the second quarry is connected. Inform the player if not.
   -- When that is finally done (and 30 seconds have passed), go on

   -- Interludium: talk about census and statistics
   census_and_statistics(cs.fields[1])

   while #plr:get_buildings("quarry") < 2 do sleep(1400) end
   o.done = true

   messages()
end

function second_quarry()
   sleep(200)

   local o = msg_box(build_second_quarry)
   second_quarry_field.immovable:remove()
   -- remove this immovable

   local cs = nil
   immovable_is_legal = function(i)
      if (i.fields[1] == second_quarry_field) or (i.fields[1] == second_quarry_field.brn) then
         cs = allow_constructionsite(i, {"quarry"})
         return cs
      elseif(i.descr.type_name == "flag") or (i.descr.type_name == "road") then
         register_immovable_as_allowed(i)
         return true
      else return false end
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(200) end

   o.done = true
   register_immovable_as_allowed(cs)
end


function census_and_statistics(field)
   sleep(15000)

   local blocker = UserInputDisabler:new()
   close_windows()

   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false

   wl.ui.MapView():abort_road_building()

   msg_box(census_and_statistics_00)
   -- Pick any empty field
   local function _pick_empty_field()
      local reg = field:region(2)
      local f
      repeat
         f = reg[math.random(#reg)]
      until not f.immovable
      return f
   end

   click_on_field(_pick_empty_field())
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.census)
   sleep(300)
   click_on_field(_pick_empty_field())
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.watch)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.statistics)

   msg_box(census_and_statistics_01)

   blocker:lift_blocks()
end

function messages()
   -- Teach the player about receiving messages
   sleep(10)

   local o = send_message(teaching_about_messages)

   while #plr.inbox > 0 do sleep(200) end
   o.done = true

   sleep(500)

   local o = msg_box(closing_msg_window_00)

   -- Wait for messages window to close
   while wl.ui.MapView().windows.messages do sleep(300) end
   o.done = true

   msg_box(closing_msg_window_01)

   sleep(800)

   destroy_quarries()
end

function destroy_quarries()
   sleep(200)
   -- Remove all rocks
   remove_all_rocks(first_quarry_field:region(6))

   -- Wait for messages to arrive
   while #plr.inbox < 2 do sleep(300) end

   local o = msg_box(destroy_quarries_message)

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true

   while #plr:get_buildings("quarry") > 0 do sleep(200) end
   o.done = true

   sleep(3000)

   expansion()
end

function expansion()
   -- Teach about expanding the territory
   sleep(10)

   local o = msg_box(introduce_expansion)

   -- wait until there are soldiers inside so that the player sees the expansion
   local soldier_inside = false
   while not soldier_inside do
      local military_buildings = array_combine(plr:get_buildings("sentry"), plr:get_buildings("donjon"), plr:get_buildings("barrier"), plr:get_buildings("fortress"), plr:get_buildings("citadel"))
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
   msg_box(military_building_finished)

   conclusion()
end

function conclusion()
   sleep(10000) -- to give the player time to see his expanded area

   -- Conclude the tutorial with final words and information
   -- on how to quit
   msg_box(conclude_tutorial)

end

run(bad_boy_sentry)
run(starting_infos)
