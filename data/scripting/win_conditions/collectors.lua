-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/formatting.lua"
include "scripting/format_scenario.lua"
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Collectors"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Collectors")
local wc_version = 2
local wc_desc = _ (
   "You get points for precious wares in your warehouses. The player with " ..
   "the highest number of wares at the end of 4 hours wins the game."
)
local wc_points = _"Points"
return {
   name = wc_name,
   description = wc_desc,
   func = function()

   -- set the objective with the game type for all players
   broadcast_objective("win_condition", wc_descname, wc_desc)

   local plrs = wl.Game().players
   local teams = {}
   for idx,plr in ipairs(plrs) do
      if (plr.team ~= 0) then
         if (teams[plr.team] == nil) then
            teams[plr.team] = {}
         end
         table.insert(teams[plr.team], plr)
      end
   end

   -- The list of wares that give points
   local point_table = {
      barbarians = {
         gold = 3,
         ax = 2,
         ax_sharp = 3,
         ax_broad = 4,
         ax_bronze = 4,
         ax_battle = 6,
         ax_warriors = 10,
         helmet = 2,
         helmet_mask = 3,
         helmet_warhelm = 6,
      },
      barbarians_order = {
         "gold", "ax", "ax_sharp", "ax_broad", "ax_bronze", "ax_battle",
         "ax_warriors", "helmet", "helmet_mask", "helmet_warhelm",
      },

      empire = {
         gold = 3,
         spear_wooden = 1,
         spear = 3,
         spear_advanced = 4,
         spear_heavy = 7,
         spear_war = 8,
         armor_helmet = 2,
         armor = 3,
         armor_chain = 4,
         armor_gilded = 8,
      },
      empire_order = {
         "gold", "spear_wooden", "spear", "spear_advanced", "spear_heavy",
         "spear_war", "armor_helmet", "armor", "armor_chain", "armor_gilded"
      },

      atlanteans = {
         gold = 3,
         trident_light = 2,
         trident_long = 3,
         trident_steel = 4,
         trident_double = 7,
         trident_heavy_double = 8,
         shield_steel = 4,
         shield_advanced = 7,
         tabard = 1,
         tabard_golden = 5,
      },
      atlanteans_order = {
         "gold", "trident_light", "trident_long", "trident_steel",
         "trident_double", "trident_heavy_double", "shield_steel",
         "shield_advanced", "tabard", "tabard_golden"
       },
   }

   -- Calculate the momentary points for a list of players
   local function _calc_points(players)
      set_textdomain("win_conditions")
      local team_points = 0
      local descr = ""

      for idx, plr in ipairs(players) do
         local bs = array_combine(
            plr:get_buildings(plr.tribe_name .. "_headquarters"), plr:get_buildings(plr.tribe_name .. "_warehouse"), plr:get_buildings(plr.tribe_name .. "_port")
         )

         descr = descr .. h2((_"Status for %s"):format(plr.name))
         local points = 0
         for idx, ware in ipairs(point_table[plr.tribe_name .. "_order"]) do
            local value = point_table[plr.tribe_name][ware]
            local count = 0
            for idx,b in ipairs(bs) do
               count = count + b:get_wares(ware)
            end
            local lpoints = count * value
            points = points + lpoints

            local warename = wl.Game():get_ware_description(ware).descname
            -- TRANSLATORS: For example: 'gold (3 P) x 4 = 12 P', P meaning 'Points'
            descr = descr .. listitem_bullet(_"%1$s (%2$i P) x %3$i = %4$i P"):bformat(
               warename, value, count, lpoints
            )
         end
         descr = descr .. "</p>" .. h3(ngettext("Total: %i point", "Total: %i points", points)):format(points)
         team_points = team_points + points
      end
      
      return team_points, descr
   end

   
   -- Send all players the momentary game state
   local function _send_state(remaining_time, plrs)
      set_textdomain("win_conditions")
      local h = math.floor(remaining_time / 60)
      local m = remaining_time % 60
      -- TRANSLATORS: Context: 'The game will end in (2 hours and) 30 minutes.'
      local time = ""
      if m > 0 then
         time = (ngettext("%i minute", "%i minutes", m)):format(m)
      end
      if h > 0 then
         if m > 0 then
            -- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
            time = (ngettext("%1% hour and %2%", "%1% hours and %2%", h)):bformat(h, time)
         else
            -- TRANSLATORS: Context: 'The game will end in 2 hours.'
            time = (ngettext("%1% hour", "%1% hours", h)):bformat(h)
         end
      end
      -- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
      local msg = p(_"The game will end in %s."):format(time)

      -- Points for players without team
      for idx, plr in ipairs(plrs) do
         if (plr.team == 0) then
            local points, pstat = _calc_points({plr})
            msg = msg .. "<p font-size=8> <br></p>" .. pstat
         end
      end
      -- Team points
      for idx, team in ipairs(teams) do
         local points, pstat = _calc_points(team)
         local message = h1((_"Status for Team %d"):format(idx))
            .. pstat
            .. h2(ngettext("Team Total: %i point", "Team Total: %i points", points)):format(points)
         msg = msg .. "<p font-size=8> <br></p>" .. message
      end

      for idx, plr in ipairs(plrs) do
         send_message(plr, game_status.title,  "<rt>" .. msg .. "</rt>", {popup = true})
      end
   end

   local function _game_over(plrs)
      local points = {}
      for idx,plr in ipairs(plrs) do
         if (plr.team == 0) then
            points[#points + 1] = { plr, _calc_points({plr}) }
         else
            points[#points + 1] = { plr, _calc_points(teams[plr.team]) }
         end
      end
      table.sort(points, function(a,b) return a[2] < b[2] end)
      for i=1,#points-1 do
         local player = points[i][1]
         player:send_message(lost_game_over.title, lost_game_over.body)
         if (player.team == 0) then
            wl.game.report_result(player, 0, make_extra_data(player, wc_descname, wc_version, {score=points[i][2]}))
         else
            wl.game.report_result(player, 0, make_extra_data(player, wc_descname, wc_version, {score=_calc_points({player}), team_score=points[i][2]}))
         end
      end
      local player = points[#points][1]
      player:send_message(won_game_over.title, won_game_over.body)
      if (player.team == 0) then
         wl.game.report_result(player, 1, make_extra_data(player, wc_descname, wc_version, {score=points[#points][2]}))
      else
         wl.game.report_result(player, 1, make_extra_data(player, wc_descname, wc_version, {score=_calc_points({player}), team_score=points[#points][2]}))
      end
   end

   -- Instantiate the hook to calculate points
   hooks.custom_statistic = {
      name = wc_points,
      pic = "images/wui/stats/genstats_points.png",
      calculator = function(p)
         local pts = _calc_points(p)
         return pts
      end,
   }

   sleep(1000)

   local remaining_time = 60 * 4

   -- Endless loop
   while true do
      _send_state(remaining_time, plrs)

      -- Game ended?
      if remaining_time <= 0 then
         _game_over(plrs)
         break
      end

      local runs = 0
      repeat
         sleep(5000)
         check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
         runs = runs + 1
      until runs >= 120 -- 120 * 5000ms = 600000 ms = 10 minutes
      remaining_time = remaining_time - 10
   end
end
}
