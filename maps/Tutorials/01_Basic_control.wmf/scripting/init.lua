-- =======================================================================
--                            Tutorial Mission
-- =======================================================================

-- ===============
-- Initialization
-- ===============
plr = wl.Game().players[1]
plr:allow_buildings("all")
map = wl.Game().map

-- A default headquarters
include "tribes/barbarians/scripting/sc00_headquarters.lua"
init.func(plr) -- defined in sc00_headquarters

set_textdomain("scenario_tutorial01.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

-- Constants
first_lumberjack_field = map:get_field(16,10)
first_quarry_field = map:get_field(8,12)
second_quarry_field = map:get_field(5,10)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
immovable_is_legal = function(i) return false end

include "map:scripting/texts.lua"

-- =================
-- Helper functions
-- =================

-- A small helper class to disable/enable autosaving and user interaction
UserInputDisabler = {}
function UserInputDisabler:new()
   local rv = {}
   setmetatable(rv, self)
   self.__index = self

   rv:establish_blocks()

   return rv
end
function UserInputDisabler:establish_blocks()
   self._ui_state = wl.ui.get_user_input_allowed()
   local game = wl.Game()
   self._as_state = game.allow_saving

   wl.ui.set_user_input_allowed(false)
   wl.Game().allow_saving = false
end
function UserInputDisabler:lift_blocks()
   wl.ui.set_user_input_allowed(self._ui_state)
   wl.Game().allow_saving = self._as_state
end

function _try_add_objective(i)
   -- Add an objective that is defined in the table i to the players objectives.
   -- Returns the new objective or nil. Does nothing if i does not specify an
   -- objective.
   local o = nil
   if i.obj_name then
      o = plr:add_objective(i.obj_name, i.obj_title, i.obj_body)
   end
   return o
end

function msg_box(i)
   wl.Game().desired_speed = 1000

   local blocker = UserInputDisabler:new()

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

   wl.ui.set_user_input_allowed(true)
   plr:message_box(i.title, i.body, i)

   blocker:lift_blocks()

   local o = _try_add_objective(i)

   sleep(130)

   return o
end

function send_message(i)
   plr:send_message(i.title, i.body, i)

   local o = _try_add_objective(i)
   sleep(130)
   return o
end

function click_on_field(f, g_T, g_sleeptime)
   local sleeptime = g_sleeptime or 500

   local blocker = UserInputDisabler:new()

   mouse_smoothly_to(f, g_T)
   sleep(sleeptime)

   wl.ui.MapView():click(f)
   sleep(sleeptime)

   blocker:lift_blocks()
end

function click_on_panel(panel, g_T, g_sleeptime)
   local sleeptime = g_sleeptime or 500

   local blocker = UserInputDisabler:new()

   sleep(sleeptime)
   if not panel.active then -- If this is a tab and already on, do nothing
      mouse_smoothly_to_panel(panel, g_T)
      sleep(sleeptime)
      if panel.press then panel:press() sleep(250) end
      if panel.click then panel:click() end
      sleep(sleeptime)
   end

   blocker:lift_blocks()
end

-- Make sure the user is in road building mode starting from the given flag
function enter_road_building_mode(flag)
   local mv = wl.ui.MapView()

   if mv.is_building_road then
      mv:abort_road_building()
   end

   mv:start_road_building(flag)
end


-- Remove all stones in a given environment. This is done
-- in a loop for a nice optical effect
function remove_all_stones(fields, g_sleeptime)
   local sleeptime = g_sleeptime or 150
   while #fields > 0 do
      local idx = math.random(#fields)
      local f = fields[idx]
      local remove_field = true

      if f.immovable then
         local n = f.immovable.descr.name:match("greenland_stones(%d*)")
         if n then
            n = tonumber(n)
            f.immovable:remove()
            if n > 1 then
               remove_field = false
               map:place_immovable("greenland_stones" .. n-1, f)
            end
            sleep(sleeptime)
         end
      end

      if remove_field then
         table.remove(fields, idx)
      end
   end
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
   if is_building(i) or i.descr.type_name == "constructionsite" then
      registered_player_immovables[_fmt(i.fields[1].brn)] = true
   end
end
register_immovable_as_allowed(map.player_slots[1].starting_field.immovable)

function bad_boy_sentry()
   local sf = map.player_slots[1].starting_field
   while not terminate_bad_boy_sentinel do
      -- Check all fields.
      local sent_msg = false
      for idx,f in ipairs(sf:region(8)) do
         if f.immovable and f.immovable.owner == plr and
               not registered_player_immovables[_fmt(f)] then

            -- Give the callback a chance to veto the deletion. Maybe
            -- we expect the player to build something at the moment
            if not immovable_is_legal(f.immovable) then
               -- scould the player
               if not sent_msg then
                  msg_box(scould_player)
                  sent_msg = true
               end

               -- Remove the object again
               f.immovable:remove()

               -- Make sure that the user is not building a road at the moment.
               wl.ui.MapView():abort_road_building()
            end
         end
      end
      sleep(1000)
   end
end

-- Allows constructionsites for the given buildings, all others are invalid
-- as is any other immovable build by the player
function allow_constructionsite(i, buildings)
   if i.descr.type_name == "constructionsite" then
      if not buildings then return i end
      for idx,n in ipairs(buildings) do
         if i.building == n then return i end
      end
      return false
   elseif i.descr.type_name == "flag" then
      local tr = i.fields[1].tln.immovable
      if tr and tr.descr.type_name == "constructionsite" then
         return allow_constructionsite(tr, buildings)
      end
   end

   return false
end

-- ================
-- Message threads
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

   sleep(30*1000) -- let the player experiment a bit with the speed
   msg_box(construction_site_window)

   while #plr:get_buildings("lumberjacks_hut") < 1 do sleep(300) end

   msg_box(lumberjack_message_07)

   learn_to_move()
end

function learn_to_move()
   -- Teaching the user how to scroll on the map
   local o = msg_box(inform_about_stones)

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
   -- TODO(codereview): Is there a possibility to check for the road (i.e. whether quarry and headquarters are in the same economy)?
   sleep(20*1000)
   
   second_quarry()

   -- Wait a while
   sleep(60*1000)
   -- check every 30 seconds if the second quarry is connected. Inform the player if not
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
   -- Remove all stones
   remove_all_stones(first_quarry_field:region(6))

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
   -- Conclude the tutorial with final words and information
   -- on how to quit

   sleep(10000) -- to give the player time to see his expanded area
   msg_box(conclude_tutorial)

end

run(bad_boy_sentry)
run(starting_infos)
