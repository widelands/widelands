-- =======================================================================
--                            Tutorial Mission
-- =======================================================================

-- ===============
-- Initialization
-- ===============
plr = wl.Game().players[1]
plr:allow_buildings("all")

-- A default headquarters
include "tribes/barbarians/scripting/sc00_headquarters_medium.lua"
init.func(plr) -- defined in sc00_headquarters_medium

set_textdomain("scenario_tutorial01.wmf")

include "scripting/coroutine.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

-- Constants
first_lumberjack_field = wl.Game().map:get_field(16,10)
first_quarry_field = wl.Game().map:get_field(8,12)
conquer_field = wl.Game().map:get_field(6,18)
trainings_ground = wl.Game().map:get_field(33,57)

-- Global variables
registered_player_immovables = {}
terminate_bad_boy_sentinel = false
illegal_immovable_found = function(i) return false end

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

function warp_houses(descriptions)
   local blocker = UserInputDisabler:new()

   for idx, d in ipairs(descriptions) do
      local name, x, y = d[1], d[2], d[3]
      mouse_smoothly_to(wl.Game().map:get_field(x, y))
      sleep(300)
      prefilled_buildings(plr, d)
      sleep(300)
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

function build_road(field, ...)
   -- Build a road by clicking the UI. A little faster than before

   -- Make sure that there is room for the road: Rip all immovables
   local cf = field
   for idx, d in ipairs{...} do
      if not (d == '|' or d == '.') then
         if cf.immovable and cf.immovable.owner ~= plr then
            cf.immovable:remove()
         end
         cf = cf[d .. 'n']
      end
   end

   mouse_smoothly_to(field, 400, 200)

   local function _start_road(field)
      wl.ui.MapView():click(field)
      click_on_panel(
         wl.ui.MapView().windows.field_action.buttons.build_road, 100, 200
      )
   end

   _start_road(field)

   for idx, d in ipairs{...} do
      if d == '|' or d == '.' then
         mouse_smoothly_to(field, 400, 200)
         wl.ui.MapView():click(field)
         if d == '|' then
            wl.ui.MapView():click(field)
            click_on_panel(
               wl.ui.MapView().windows.field_action.buttons.build_flag, 100, 200
            )
            _start_road(field)
         end
      else
         field = field[d .. 'n']
      end
   end
end

function build_eastern_trainings_area(citadel_field)
   -- Build some infrastructure as another example
   local blocker = UserInputDisabler:new()

   plr:reveal_fields(citadel_field:region(8))
   scroll_smoothly_to(wl.Game().map:get_field(21,9))
   scroll_smoothly_to(citadel_field)

   warp_houses{
      {"fortress", 31, 63, soldiers = {[{3,5,0,2}] = 8 }},
      {"warehouse", 33, 57,
         soldiers = {
            [{0,0,0,0}] = 1,
            [{1,0,0,0}] = 1,
            [{2,0,0,0}] = 1,
            [{3,0,0,0}] = 1,
            [{0,1,0,0}] = 1,
            [{0,2,0,0}] = 1,
            [{0,3,0,0}] = 1,
            [{0,4,0,0}] = 1,
            [{0,5,0,0}] = 1,
            [{0,0,0,1}] = 1,
            [{0,0,0,2}] = 1,
            [{3,5,0,2}] = 30,
         },
         workers = {
            builder = 1
         },
         wares = {
            log = 40,
            blackwood = 40,
            cloth = 10,
            gold = 10,
            grout = 30,
            raw_stone = 30,
            thatchreed = 40,
         }
      },
      {"trainingcamp", 31, 56, soldiers = {} },
      {"sentry", 28, 57, soldiers = {[{3,5,0,2}] = 2 }},
      {"sentry", 37, 61, soldiers = {[{3,5,0,2}] = 2 }},
   }
   -- Build the roads
   local map = wl.Game().map
   build_road(map:get_field(31,57), "bl", "bl", "|", "br", "br", "|",
      "r", "r", "|", "tr", "tr", "tl", ".")
   build_road(map:get_field(29,58), "r", "br", ".")
   build_road(map:get_field(38,62), "l", "l", "|", "l", "bl",
      "|", "tl", "tl", ".")
   build_road(map:get_field(32, 0), "tr", "tr", "tr", '.')

   -- Add wares to the trainingssite so that it does something. Also
   -- add buildwares to the warehouse
   local ts = map:get_field(31,56).immovable
   ts:set_wares(ts.valid_wares)

   scroll_smoothly_to(citadel_field)

   blocker:lift_blocks()
end

-- Remove all stones in a given environment. This is done
-- in a loop for a nice optical effect
function remove_all_stones(fields, g_sleeptime)
   local sleeptime = g_sleeptime or 150
   local map = wl.Game().map
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
               map:place_immovable("stones" .. n-1, f)
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
   if i.building_type or i.type == "constructionsite" then
      registered_player_immovables[_fmt(i.fields[1].brn)] = true
   end
end
register_immovable_as_allowed(wl.Game().map.player_slots[1].starting_field.immovable)

function bad_boy_sentry()
   local sf = wl.Game().map.player_slots[1].starting_field
   while not terminate_bad_boy_sentinel do
      -- Check all fields.
      local sent_msg = false
      for idx,f in ipairs(sf:region(8)) do
         if f.immovable and f.immovable.owner == plr and
               not registered_player_immovables[_fmt(f)] then

            -- Give the callback a chance to veto the deletion. Maybe
            -- we expect the player to build something at the moment
            if not illegal_immovable_found(f.immovable) then
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
   if i.type == "constructionsite" then
      if not buildings then return i end
      for idx,n in ipairs(buildings) do
         if i.building == n then return i end
      end
      return false
   elseif i.type == "flag" then
      local tr = i.fields[1].tln.immovable
      if tr and tr.type == "constructionsite" then
         return allow_constructionsite(tr, buildings)
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
   illegal_immovable_found = function(i) return true end

   msg_box(lumberjack_message_01)

   local blocker = UserInputDisabler:new()

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

   click_on_field(wl.Game().map.player_slots[1].starting_field.brn)

   msg_box(lumberjack_message_04)

   register_immovable_as_allowed(first_lumberjack_field.immovable) -- hut + flag

   local f = wl.Game().map:get_field(14,11)
   register_immovable_as_allowed(f.immovable) -- road + everything on it

   illegal_immovable_found = function(i) return false end

   blocker:lift_blocks()

   sleep(15000)

   local o = msg_box(lumberjack_message_05)

   local blocker = UserInputDisabler:new()

   local f = wl.Game().map:get_field(14,11)
   scroll_smoothly_to(f)
   mouse_smoothly_to(f)

   blocker:lift_blocks()

   -- Wait for flag
   while not (f.immovable and f.immovable.type == "flag") do sleep(300) end
   o.done = true

   sleep(300)

   msg_box(lumberjack_message_06)

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

   msg_box(congratulate_and_on_to_quarry)

   build_a_quarry()
end

function build_a_quarry()
   sleep(200)

   -- Teaching how to build a quarry and the nits and knacks of road building.
   local o = msg_box(order_quarry_recap_how_to_build)

   local cs = nil
   -- Wait for the constructionsite to come up.
   illegal_immovable_found = function(i)
      cs = allow_constructionsite(i, {"quarry"})
      return cs
   end

   -- Wait for the constructionsite to be placed
   while not cs do sleep(200) end
   o.done = true
   register_immovable_as_allowed(cs)

   enter_road_building_mode(cs.fields[1].brn.immovable)

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

   local blocker = UserInputDisabler:new()

   illegal_immovable_found = function() return true end

   msg_box(talk_about_roadbuilding_00)
   -- Showoff one-by-one roadbuilding
   local map = wl.Game().map
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

   -- From now on, the player can build whatever he wants
   terminate_bad_boy_sentinel = true

   -- Wait a while
   sleep( 100*1000 )

   -- Interludium: talk about census and statistics
   census_and_statistics(cs.fields[1])

   while #plr:get_buildings("quarry") < 1 do sleep(1400) end
   o.done = true

   messages()
end

function census_and_statistics(field)
   sleep(15000)

   local blocker = UserInputDisabler:new()

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

   -- Remove all stones
   remove_all_stones(first_quarry_field:region(6))

   -- Wait for message to arrive
   while #plr.inbox < 1 do sleep(300) end

   sleep(800)
   msg_box(conclude_messages)

   sleep(3000)
   expansion()
end

function expansion()
   -- Teach about expanding your territory.
   sleep(10)

   -- This is not really needed since the stones are already removed, but if
   -- we're debugging and we start with this function it is most useful to have
   -- the stones away already
   remove_all_stones(first_quarry_field:region(6), 20)

   local o = msg_box(introduce_expansion)

   while not conquer_field.owner do sleep(100) end
   o.done = true

   mining()
end

function mining()
   -- Teach about geologist and resources
   sleep(10)

   msg_box(mining_00)

   local function _find_good_flag_position()
      fields = conquer_field:region(8)
      while #fields > 0 do
         local idx = math.random(#fields)
         local f = fields[idx]

         if f.terr:match("berg%d+") and f.terd:match("berg%d+") then
            if pcall(function() plr:place_flag(f) end) then
               f.immovable:remove()
               return f
            end
         end

         table.remove(fields, idx)
      end
   end

   local function _find_nearby_flag()
      for i=2,8 do
         for idx, f in ipairs(conquer_field:region(i)) do
            if f.immovable and f.immovable.type == "flag" then
               return f
            end
         end
      end
   end

   scroll_smoothly_to(conquer_field)

   local dest = _find_good_flag_position()
   local start = _find_nearby_flag()

   -- Build a road, call a geologist
   click_on_field(start)
   click_on_panel(wl.ui.MapView().windows.field_action.tabs.roads)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_road)
   click_on_field(dest)
   click_on_field(dest) -- second click
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.build_flag)
   click_on_field(dest)
   click_on_panel(wl.ui.MapView().windows.field_action.buttons.geologist)

   sleep(6000)

   msg_box(mining_01)

   local function _wait_for_some_resi(wanted)
      while 1 do
         local cnt = 0
         for idx, f in ipairs(dest:region(6)) do
            if f.immovable and f.immovable.name:sub(1,4) == "resi" then
               cnt = cnt + 1
               if cnt >= wanted then return end
            end
         end
         sleep(500)
      end
   end
   _wait_for_some_resi(8)

   scroll_smoothly_to(dest)

   msg_box(mining_02)

   training()
end

function training()
   -- Teach about trainingsites and soldiers
   sleep(300)

   msg_box(warfare_and_training_00)

   local citadel_field = wl.Game().map:get_field(31, 63)

   build_eastern_trainings_area(citadel_field)
   sleep(8000)

   msg_box(warfare_and_training_01)

   sleep(5000)
   scroll_smoothly_to(citadel_field)

   local o = msg_box(enhance_fortress)
   while not (citadel_field.immovable and
      citadel_field.immovable.name == "citadel") do sleep(800) end
   o.done = true

   -- Wait for soldiers to move in
   local citadel = citadel_field.immovable
   local break_out = false
   while not break_out do
      for k,v in pairs(citadel:get_soldiers("all")) do
         break_out = true
         break -- Break out if there is at least one soldier here
      end

      sleep(500)
   end

   -- Create enemy tribe
   prefilled_buildings(wl.Game().players[2],
      {"barrier", 25, 6},
      {"sentry", 29, 16},
      {"tower", 30, 21},
      {"headquarters", 30, 27,
         workers = {
            carrier = 50,
         },
         soldiers = {
            [{0,0,0,0}] = 15,
         }
      }
   )

   scroll_smoothly_to(citadel_field)
   local o = msg_box(attack_enemey)

   local plr2 = wl.Game().players[2]
   while #plr2:get_buildings("headquarters") > 0 or not plr2.defeated do
      sleep(3000)
   end
   o.done = true

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
