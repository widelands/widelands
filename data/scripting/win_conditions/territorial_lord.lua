-- =======================================================================
--                   Territorial Lord Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"
include "scripting/win_conditions/territorial_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Territorial Lord"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Territorial Lord")
local wc_version = 2
local wc_desc = _ (
   "Each player or team tries to obtain more than half of the map’s " ..
   "area. The winner will be the player or the team that is able to keep " ..
   "that area for at least 20 minutes."
)
return {
   name = wc_name,
   description = wc_desc,
   func = function()
      local plrs = wl.Game().players

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

      -- Get all valueable fields of the map
      local fields = get_buildable_fields()

      -- these variables will be used once a player or team owns more than half
      -- of the map's area
      local currentcandidate = "" -- Name of Team or Player
      local candidateisteam = false
      local remaining_time = 10 -- (dummy) -- time in secs, if == 0 -> victory

      -- Find all valid teams
      local teamnumbers = get_teamnumbers(plrs) -- array with team numbers

      local all_player_points = {}

      local function _calc_points()
         local teampoints = {}     -- points of teams
         local maxplayerpoints = 0 -- the highest points of a player without team
         local maxpointsplayer = 0 -- the player
         local foundcandidate = false

         all_player_points = count_owned_fields_for_all_players(fields, plrs)

         for idx, p in ipairs(plrs) do
            local team = p.team
            if team == 0 then
               if maxplayerpoints < all_player_points[p.number] then
                  maxplayerpoints = all_player_points[p.number]
                  maxpointsplayer = p
               end
            else
               if not teampoints[team] then -- init the value
                  teampoints[team] = 0
               end
               teampoints[team] = teampoints[team] + all_player_points[p.number]
            end
         end

         if maxplayerpoints > ( #fields / 2 ) then
            -- player owns more than half of the map's area
            foundcandidate = true
            if candidateisteam == false and currentcandidate == maxpointsplayer.name then
               remaining_time = remaining_time - 30
            else
               currentcandidate = maxpointsplayer.name
               candidateisteam = false
               remaining_time = 20 * 60 -- 20 minutes
            end
         else
            for idx, t in ipairs(teamnumbers) do
               if teampoints[t] > ( #fields / 2 ) then
                  -- this team owns more than half of the map's area
                  foundcandidate = true
                  if candidateisteam == true and currentcandidate == t then
                     remaining_time = remaining_time - 30
                  else
                     currentcandidate = t
                     candidateisteam = true
                     remaining_time = 20 * 60 -- 20 minutes
                  end
               end
            end
         end
         if not foundcandidate then
            currentcandidate = ""
            candidateisteam = false
            remaining_time = 10
         end
      end

      local function _send_state()
         set_textdomain("win_conditions")
         local candidate = currentcandidate
         if candidateisteam then
            candidate = (_"Team %i"):format(currentcandidate)
         end
         local msg1 = p(_"%s owns more than half of the map’s area."):format(candidate)
         msg1 = msg1 .. p(ngettext("You’ve still got %i minute to prevent a victory.",
                   "You’ve still got %i minutes to prevent a victory.",
                   remaining_time / 60))
               :format(remaining_time / 60)

         local msg2 = p(_"You own more than half of the map’s area.")
         msg2 = msg2 .. p(ngettext("Keep it for %i more minute to win the game.",
                   "Keep it for %i more minutes to win the game.",
                   remaining_time / 60))
               :format(remaining_time / 60)

         for idx, player in ipairs(plrs) do
            if candidateisteam and currentcandidate == player.team
               or not candidateisteam and currentcandidate == player.name then
               send_message(player, game_status.title, msg2, {popup = true})
            else
               send_message(player, game_status.title, msg1, {popup = true})
            end
         end
      end

      -- Start a new coroutine that checks for defeated players
      run(function()
         while remaining_time ~= 0 do
            sleep(5000)
            check_player_defeated(plrs, lost_game.title, lost_game.body, wc_descname, wc_version)
         end
      end)

      -- here is the main loop!!!
      while true do
         -- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
         sleep(30000)

         -- Check if a player or team is a candidate and update variables
         _calc_points()

         -- Do this stuff, if the game is over
         if remaining_time == 0 then
            for idx, p in ipairs(plrs) do
               p.see_all = 1
               if candidateisteam and currentcandidate == p.team
                  or not candidateisteam and currentcandidate == p.name then
                  p:send_message(won_game_over.title, won_game_over.body)
                  wl.game.report_result(p, 1, make_extra_data(p, wc_descname, wc_version, {score=all_player_points[p.number]}))
               else
                  p:send_message(lost_game_over.title, lost_game_over.body)
                  wl.game.report_result(p, 0, make_extra_data(p, wc_descname, wc_version, {score=all_player_points[p.number]}))
               end
            end
            break
         end

         -- If there is a candidate, check whether we have to send an update
         if remaining_time % 300 == 0 then -- every 5 minutes (5 * 60 )
            _send_state()
         end
      end
   end
}
