-- =======================================================================
--                       Wood Gnome win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Wood Gnome"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Wood Gnome")
local wc_version = 2
local wc_desc_placeholder = _(
[[As wood gnome you like big forests, so your task is to have more trees on ]] ..
[[your territory than any other player. The game will end after %s. ]] ..
[[The one with the most trees at that point will win the game.]])
-- TRANSLATORS: Will be inserted into "The game will end after %s."
local wc_desc = wc_desc_placeholder:bformat(_("the configured time limit"))

local wc_trees_owned = "Trees owned"
-- This needs to be exactly like wc_trees_owned, but localized, because wc_trees_owned
-- will be used as the key to fetch the translation in C++
local wc_trees_owned_i18n = _("Trees owned")

local r = {
   name = wc_name,
   description = wc_desc,
   configurable_time = true,
   peaceful_mode_allowed = true,
   init = function()
      -- Calculate valuable fields
      wl.Game().map:count_terrestrial_fields()
   end,
   func = function()
   local plrs = wl.Game().players
   local game = wl.Game()

   -- set the maximum game time of 4 hours
   local max_time = game.win_condition_duration

   -- set the objective with the game type for all players
   broadcast_objective("win_condition", wc_descname, wc_desc_placeholder:bformat(format_remaining_raw_time(max_time)))

   -- The function to calculate the current points.
   local _last_time_calculated = -100000
   local playerpoints = {}
   local function _calc_points()

      if _last_time_calculated > game.time - 5000 then
         return
      end

      playerpoints = count_owned_valuable_fields_for_all_players(plrs, "tree")
      _last_time_calculated = game.time
   end

   local function _send_state(remaining_time, plrs, show_popup)
      _calc_points()
      local msg = format_remaining_time(remaining_time) .. msg_vspace() .. game_status.body

      push_textdomain("win_conditions")
      for idx,plr in ipairs(plrs) do
         local trees = (ngettext ("%i tree", "%i trees", playerpoints[plr.number]))
               :format(playerpoints[plr.number])
         -- TRANSLATORS: %1$s = player name, %2$s = x tree(s)
         msg = msg .. p(_("%1$s has %2$s at the moment.")):bformat(plr.name,trees)
      end
      pop_textdomain()

      broadcast(plrs, game_status.title, msg, {popup = show_popup})
   end

   local function _game_over(plrs)
      _calc_points()
      local points = {}
      for idx,plr in ipairs(plrs) do
         points[#points + 1] = { plr, playerpoints[plr.number] }
      end
      table.sort(points, function(a,b) return a[2] < b[2] end)

      push_textdomain("win_conditions")
      local msg = msg_vspace() .. game_status.body
      for idx,plr in ipairs(plrs) do
         msg = msg .. msg_vspace()
         local trees = (ngettext ("%i tree", "%i trees", playerpoints[plr.number])):format(playerpoints[plr.number])
         -- TRANSLATORS: %1$s = player name, %2$s = x tree(s)
         msg = msg ..  p(_("%1$s had %2$s.")):bformat(plr.name,trees)
      end
      msg = msg .. msg_vspace()
      local trees = (ngettext ("%i tree", "%i trees", points[#points][2])):format(points[#points][2])
      if #points == 1 or points[#points][2] ~= points[#points - 1][2] then -- one winner
         -- TRANSLATORS: %1$s = player name, %2$s = x tree(s)
         msg = msg ..  h3(_("The winner is %1$s with %2$s.")):bformat(points[#points][1].name, trees)
      else
         -- TRNANSLATORS: %s = x tree(s)
         msg = msg .. h3(_("The winners had %s.")):bformat(trees)
         -- TODO maybe, create list of winners and show as %$1s: The winners are %$1s with %$2s.
      end
      pop_textdomain()

      local win_points = points[#points][2] -- points of winner(s)
      local privmsg = ""
      for i=1,#points do
         local win_lost = 0
         local end_msgs = lost_game_over
         if points[i][2] >= win_points then
            end_msgs = won_game_over
            win_lost = 1
         end
         privmsg = end_msgs.body
         privmsg = privmsg .. msg
         points[i][1]:send_to_inbox(end_msgs.title, privmsg)
         wl.game.report_result(points[i][1], win_lost, make_extra_data(points[i][1], wc_descname, wc_version, {score=points[i][2]}))
      end
   end

   -- Install statistics hook
   hooks.custom_statistic = {
      name = wc_trees_owned,
      pic = "images/wui/stats/genstats_trees.png",
      calculator = function(p)
         _calc_points(p)
         return playerpoints[p.number] or 0
      end,
   }

   -- Start a new coroutine that triggers status notifications.
   run(function()
      local remaining_time = max_time
      while game.time <= ((max_time - 5) * 60 * 1000) and count_factions(plrs) > 1 do
         remaining_time, show_popup = notification_remaining_time(max_time, remaining_time)
         _send_state(remaining_time, plrs, show_popup)
      end
   end)

   -- main loop checks for defeated players
   while game.time < (max_time * 60 * 1000) and count_factions(plrs) > 1 do
      sleep(1000)
      check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
   end

   -- Game has ended
   _game_over(plrs)

end
}
pop_textdomain()
return r
