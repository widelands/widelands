-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("scenario_tutorial.wmf")

-- ===============
-- Initialization
-- ===============
plr = wl.game.Player(1)
plr:allow_buildings("all")

-- A default headquarters
use("tribe_barbarians", "sc00_headquarters_medium")
init.func(plr) -- defined in sc00_headquarters_medium


use("aux", "coroutine")
use("aux", "ui")
use("aux", "table")

-- Constants
first_lumberjack_field = wl.map.Field(16,10)
first_quarry_field = wl.map.Field(8,12)
conquer_field = wl.map.Field(19,15)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
illegal_immovable_found = function(i) return false end

use("map", "texts")


-- TODO: add objectives, there are none currently

-- =================
-- Helper functions 
-- =================
function msg_box(i)
   wl.game.set_speed(1000)

   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)

      i.field = nil -- Otherwise message box jumps back
   end

   if i.pos == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.pos == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   plr:message_box(i.title, i.body, i)

   sleep(130)
end

function send_message(i)
   plr:send_message(i.title, i.body, i)

   sleep(130)
end
   
function click_on_field(f, g_T)
   sleep(500)
   mouse_smoothly_to(f, g_T)
   sleep(500)
   wl.ui.MapView():click(f)
   sleep(500)
end

function click_on_panel(panel, g_T)
   sleep(500)
   if not panel.active then -- If this is a tab and already on, do nothing
      mouse_smoothly_to_panel(panel, g_T)
      sleep(500)
      if panel.press then panel:press() sleep(250) end
      if panel.click then panel:click() end
      sleep(500)
   end
end

-- ===================
-- Block mouse thread 
-- ===================
-- Keep the mouse at a given position without given the user a chance to mouse
-- away
function block_mouse_for(time) 
   local wait_till = wl.game.get_time() + time
   local x = wl.ui.MapView().mouse_position_x
   local y = wl.ui.MapView().mouse_position_y
   
   function _thread()
      while wl.game.get_time() < wait_till do
         print "Freezing!"

         wl.ui.MapView().mouse_position_x = x
         wl.ui.MapView().mouse_position_y = y
         sleep(200)
      end
   end
   run(_thread)
end

-- ==============
-- Sentry Thread 
-- ==============
-- This thread makes sure that the player does not build stuff where he
-- is not supposed to. He gets a message box when he tries and what he build
-- gets immediately ripped. This thread can be disabled temporarily.
function _fmt(f) return ("%i_%i"):format(f.x, f.y) end
function register_immovable_as_allowed(i)
   for idx, f in ipairs(i.fields) do
      registered_player_immovables[_fmt(f)] = true
   end

    -- buildings and constructionsite have a flag
   if i.building_type or i.type == "constructionsite" then
      registered_player_immovables[_fmt(i.fields[1].brn)] = true
   end
end
register_immovable_as_allowed(plr.starting_field.immovable)

function bad_boy_sentry()
   while not terminate_bad_boy_sentinel do
      -- Check all fields.
      local sent_msg = false
      for idx,f in ipairs(plr.starting_field:region(8)) do
         if f.immovable and f.immovable.player == plr and
               not registered_player_immovables[_fmt(f)] then

            -- Give the callback a chance to veto the deletion. Maybe
            -- we expect the player to build something at the moment
            if not illegal_immovable_found(f.immovable) then
               print ("Killing", f.x .. '_' .. f.y)

               -- scould the player
               if not sent_msg then
                  msg_box(scould_player)
                  sent_msg = true
               end

               -- Remove the object again
               f.immovable:remove()
            end
         end
      end
      sleep(1000)
   end
end
   
-- TODO: this needs to be done better, but a wrapping of the constructionsite
-- is needed for this. 
-- TODO: check that the constructionsite is indeed for the correct building
function allow_constructionsite(i)
   if i.type == "constructionsite" then
      return i
   elseif i.type == "flag" then
      local tr = i.fields[1].tln.immovable
      if tr and tr.type == "constructionsite" then
         return tr
      end
   end
   return false
end

-- ================
-- Message threads 
-- ================
function starting_infos()
   sleep(100)

   msg_box(initial_message_01)
   sleep(500)
   msg_box(initial_message_02)

   -- Wait for buildhelp to come on
   while not wl.ui.MapView().buildhelp do
      sleep(200)
   end
   sleep(500)

   build_lumberjack()
end

function build_lumberjack()
   sleep(100)

   -- We take control, everything that we build is legal
   illegal_immovable_found = function(i) return true end

   msg_box(lumberjack_message_01)

   mouse_smoothly_to(first_lumberjack_field)
   sleep(500)
   msg_box(lumberjack_message_02)

   sleep(500)

   click_on_field(first_lumberjack_field)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.small)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.lumberjacks_hut)

   sleep(500)
   msg_box(lumberjack_message_03)
   sleep(500)

   click_on_field(plr.starting_field.brn)

   msg_box(lumberjack_message_04)
   
   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = wl.map.Field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   illegal_immovable_found = function(i) return false end

   sleep(15000)
   
   msg_box(lumberjack_message_05)
   
   local f = wl.map.Field(14,11)
   mouse_smoothly_to(f)
   
   -- Wait for flag
   while not (f.immovable and f.immovable.type == "flag") do sleep(300) end


   sleep(300)
   
   msg_box(lumberjack_message_06)
   
   while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end

   msg_box(lumberjack_message_07)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   msg_box(inform_about_stones)
   
   function _wait_for_move()
      local cx = wl.ui.MapView().viewpoint_x
      local cy = wl.ui.MapView().viewpoint_y
      while cx == wl.ui.MapView().viewpoint_x and
            cy == wl.ui.MapView().viewpoint_y do
         sleep(300)
      end
   end

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more

   msg_box(tell_about_right_drag_move)

   _wait_for_move()
   sleep(3000) -- Give the player a chance to try this some more
   
   msg_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end
   
function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   msg_box(order_quarry_recap_how_to_build)

   local cs = nil
   -- Wait for the constructionsite to come up.
   illegal_immovable_found = function(i)
      cs = allow_constructionsite(i, "quarry")
      return cs
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(300) end
   register_immovable_as_allowed(cs)

   local function _rip_road()
      for idx,f in ipairs(cs.fields[1].brn:region(2)) do
         if f.immovable and f.immovable.type == "road" then 
            click_on_field(f)
            click_on_panel(wl.ui.MapView().windows.
               field_action.buttons.destroy_road, 300)
            sleep(200)
            return 
         end
      end
   end

   illegal_immovable_found = function() return true end

   msg_box(talk_about_roadbuilding_00)
   -- Showoff one-by-one roadbuilding
   click_on_field(wl.map.Field(9,12))
   click_on_field(wl.map.Field(10,12))
   click_on_field(wl.map.Field(11,12))
   click_on_field(wl.map.Field(12,12))
   click_on_field(wl.map.Field(12,11))
   
   block_mouse_for(3000)
   sleep(3000)

   _rip_road()
   
   msg_box(talk_about_roadbuilding_01)
   -- Showoff direct roadbuilding
   click_on_field(cs.fields[1].brn)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road, 300)
   click_on_field(plr.starting_field.brn)
   
   block_mouse_for(3000)
   sleep(3000)

   _rip_road()


   msg_box(talk_about_roadbuilding_02)
   
   -- Wait till a road is placed to the constructionsite
   local found_road = false

   illegal_immovable_found = function(i) 
      if i.type == "road" then
         local on_cs = i.start_flag == cs.fields[1].brn.immovable or
            i.end_flag == cs.fields[1].brn.immovable
         local on_hq = i.start_flag == plr.starting_field.brn.immovable or
            i.end_flag == plr.starting_field.brn.immovable

         if on_cs and on_hq then 
            register_immovable_as_allowed(i)
            found_road = true
            return true
         end
      end
      return false
   end

   while not found_road do sleep(300) end
   illegal_immovable_found = function() return false end

   -- Interludium: talk about census and statistics
   census_and_statistics(cs)

   while #plr:get_buildings("quarry") < 1 do sleep(1400) end

   messages()
end
   
function census_and_statistics(cs)
   sleep(25000)
   
   wl.ui.MapView().census = false
   wl.ui.MapView().statistics = false
  
   msg_box(census_and_statistics_00)
   -- Pick any empty field
   local function _pick_empty_field()
      local reg = cs.fields[1]:region(2)
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

end

function messages()
   -- Teach the player about receiving messages
   sleep(10)

   send_message(teaching_about_messages)

   while #plr.inbox > 0 do sleep(200) end

   msg_box(closing_msg_window_00)

   -- Wait for messages window to close
   while wl.ui.MapView().windows.messages do sleep(300) end
   
   msg_box(closing_msg_window_01)

   -- Remove all stones
   local fields = first_quarry_field:region(6)
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         local n = f.immovable.name:match("stones(%d*)")
         if n then
            n = tonumber(n)
            f.immovable:remove()
            if n > 1 then 
               remove_field = false
               wl.map.create_immovable("stones" .. n-1, f)
               sleep(150)
            end
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end

   -- Wait for message to arrive
   while #plr.inbox < 1 do sleep(300) end

   sleep(800)
   msg_box(conclude_messages)

   expansion()
end

function expansion() 
   -- Teach about expanding your territory.
   sleep(10)

   msg_box(introduce_expansion)

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true
   
   while #conquer_field.owners < 1 do sleep(100) end

   conclusion()

end

function conclusion()
   -- Conclude the tutorial with final words and information
   -- on how to quit

   sleep(4000)
   msg_box(conclude_tutorial)

end

run(bad_boy_sentry)
run(starting_infos)
