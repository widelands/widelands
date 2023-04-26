-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Collectors"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Collectors")
local wc_version = 2
local wc_desc_placeholder = _(
   "You get points for precious wares in your warehouses. The player with " ..
   "the highest score after %s wins the game."
)
-- TRANSLATORS: Will be inserted into "The player with the highest score after %s wins the game."
local wc_desc = wc_desc_placeholder:bformat(_("the configured time limit"))
local wc_points = "Points"
-- This needs to be exactly like wc_points, but localized, because wc_points
-- will be used as the key to fetch the translation in C++
local wc_points_i18n = _("Points")
local r = {
   name = wc_name,
   description = wc_desc,
   configurable_time = true,
   peaceful_mode_allowed = true,
   func = function()

   local game = wl.Game()

   -- set the maximum game time of 4 hours
   local max_time = game.win_condition_duration

   -- set the objective with the game type for all players
   broadcast_objective("win_condition", wc_descname, wc_desc_placeholder:bformat(format_remaining_raw_time(max_time)))

   local plrs = wl.Game().players
   local function _gather_teams(plrs)
   local teams = {}
      for idx,plr in ipairs(plrs) do
         if (plr.team ~= 0) then
            if (teams[plr.team] == nil) then
               teams[plr.team] = {}
            end
            table.insert(teams[plr.team], plr)
         end
      end
      return teams
   end

   -- Calculate the momentary points for a list of players
   local function _calc_points(players)
      push_textdomain("win_conditions")
      local team_points = 0
      local descr = ""

      for idx, plr in ipairs(players) do
         local bs = {}
         for name,allowed in pairs(plr.allowed_buildings) do
            if wl.Game():get_building_description(name).type_name == "warehouse" then
               bs = array_combine(bs, plr:get_buildings(name))
            end
         end

         descr = descr .. h2((_("Status for %s")):format(plr.name))
         local points = 0
         for idx,pair in ipairs(plr.tribe.collectors_points_table) do
            local count = 0
            for idx,b in ipairs(bs) do
               count = count + b:get_wares(pair.ware)
            end
            local lpoints = count * pair.points
            points = points + lpoints

            local warename = wl.Game():get_ware_description(pair.ware).descname
            -- TRANSLATORS: For example: 'gold (3 P) x 4 = 12 P', P meaning 'Points'
            descr = descr .. li(_("%1$s (%2$i P) x %3$i = %4$i P")):bformat(
               warename, pair.points, count, lpoints
            )
         end
         descr = descr .. h3(ngettext("Total: %i point", "Total: %i points", points)):format(points)
         team_points = team_points + points
      end

      pop_textdomain()
      return team_points, descr
   end

   -- Send all players the momentary game state
   local function _send_state(remaining_time, plrs, show_popup)
      local msg = format_remaining_time(remaining_time) .. msg_vspace() .. game_status.body

      -- Points for players without team
      for idx, plr in ipairs(plrs) do
         if (plr.team == 0) then
            local points, pstat = _calc_points({plr})
            msg = msg .. msg_vspace() .. pstat
         end
      end
      -- Team points
      push_textdomain("win_conditions")
      for idx, team in ipairs(_gather_teams(plrs)) do
         local points, pstat = _calc_points(team)
         local message = h1((_("Status for Team %d")):format(idx))
            .. pstat
            .. h2(ngettext("Team Total: %i point", "Team Total: %i points", points)):format(points)
         msg = msg .. msg_vspace() .. message
      end
      pop_textdomain()

      broadcast(plrs, game_status.title, msg, {popup = show_popup})
   end

   local function _game_over(plrs)
      local points = {}
      local win_points = 0
      local teams = _gather_teams(plrs)
      for idx,plr in ipairs(plrs) do
         local player_points, pstat = _calc_points({plr})
         if (plr.team == 0) then
            points[#points + 1] = { plr, player_points, player_points }
         else
            local team_points, tstat = _calc_points(teams[plr.team])
            points[#points + 1] = { plr, team_points, player_points }
         end
         if (points[#points][2] > win_points) then
            win_points = points[#points][2]
         end
      end
      for idx,info in ipairs(points) do
         local player = info[1]
         local lost_or_won = 0
         if (info[2] < win_points) then
            lost_or_won = 0
            send_to_inbox(player, lost_game_over.title, lost_game_over.body, {popup = true})
         else
            lost_or_won = 1
            send_to_inbox(player, won_game_over.title, won_game_over.body, {popup = true})
         end
         if (count_factions(plrs) > 1) then
            if (player.team == 0) then
               wl.game.report_result(player, lost_or_won, make_extra_data(player, wc_descname, wc_version, {score=info[2]}))
            else
               wl.game.report_result(player, lost_or_won, make_extra_data(player, wc_descname, wc_version, {score=info[3], team_score=info[2]}))
            end
         else
            wl.game.report_result(player, lost_or_won)
         end
      end
   end

   -- Instantiate the hook to calculate points
   hooks.custom_statistic = {
      name = wc_points,
      pic = "images/wui/stats/genstats_points.png",
      calculator = function(p)
         local pts = _calc_points({p})
         return pts
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
