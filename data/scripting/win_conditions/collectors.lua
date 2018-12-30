-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
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

      frisians = {
         gold = 3,
         sword_short = 2,
         sword_long = 3,
         sword_broad = 6,
         sword_double = 7,
         helmet = 2,
         helmet_golden = 7,
         fur_garment = 2,
         fur_garment_studded = 3,
         fur_garment_golden = 6,
      },
      frisians_order = {
         "gold", "sword_short", "sword_long", "sword_broad", "sword_double",
         "helmet", "helmet_golden", "fur_garment", "fur_garment_studded", "fur_garment_golden"
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
            descr = descr .. li(_"%1$s (%2$i P) x %3$i = %4$i P"):bformat(
               warename, value, count, lpoints
            )
         end
         descr = descr .. h3(ngettext("Total: %i point", "Total: %i points", points)):format(points)
         team_points = team_points + points
      end

      return team_points, descr
   end

   -- variable to track the maximum 4 hours of gametime
   local remaining_max_time = 4 * 60 * 60 -- 4 hours
   
   -- Send all players the momentary game state
   local function _send_state(remaining_max_time, plrs)
      local msg = vspace(8)
      set_textdomain("win_conditions")
      if remaining_max_time <= 0 then
         msg = p(_"The game has ended.") .. vspace(8)
      else
         local h = math.floor(remaining_max_time / 3600)
         local m = (remaining_max_time / 60) % 60
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
         msg = p(_"The game will end in %s."):format(time) .. vspace(8)
      end

      -- Points for players without team
      for idx, plr in ipairs(plrs) do
         if (plr.team == 0) then
            local points, pstat = _calc_points({plr})
            msg = msg .. vspace(8) .. pstat
         end
      end
      -- Team points
      for idx, team in ipairs(teams) do
         local points, pstat = _calc_points(team)
         local message = h1((_"Status for Team %d"):format(idx))
            .. pstat
            .. h2(ngettext("Team Total: %i point", "Team Total: %i points", points)):format(points)
         msg = msg .. vspace(8) .. message
      end

      for idx, plr in ipairs(plrs) do
         send_message(plr, game_status.title, msg, {popup = true})
      end
   end

   local function _game_over(plrs)
      local points = {}
      local win_points = 0
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
            player:send_message(lost_game_over.title, lost_game_over.body)
         else
            lost_or_won = 1
            player:send_message(won_game_over.title, won_game_over.body)
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

   -- main loop
   while remaining_max_time > 0 and count_factions(plrs) > 1 do
      -- Sleep 5 seconds
      sleep(5000)
      remaining_max_time = remaining_max_time - 5
      
      -- A player might have been defeated since the last calculation
      check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)

      -- at the beginning send remaining max time message only each 30 minutes
      -- if only 30 minutes or less are left, send each 5 minutes
      if ((remaining_max_time < (30 * 60) and remaining_max_time % (5 * 60) == 0)
            or remaining_max_time % (30 * 60) == 0) then
         _send_state(remaining_max_time, plrs)
      end
   end
   
   -- Game ended?
   _game_over(plrs)
end
}
