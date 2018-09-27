-- =======================================================================
--                   Territorial Time Win condition
-- =======================================================================

-- TODO(sirver): There is so much code duplication with territorial_lord.lua in
-- here. Pull that out into a separate script and reuse.

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"
include "scripting/win_conditions/territorial_functions.lua"

set_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Territorial Time"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Territorial Time")
local wc_version = 1
local wc_desc = _ (
   "Each player or team tries to obtain more than half of the map’s " ..
   "area. The winner will be the player or the team that is able to keep " ..
   "that area for at least 20 minutes, or the one with the most territory " ..
   "after 4 hours, whichever comes first."
)
local wc_has_territory = _"%1$s has %2$3.0f%% of the land (%3$i of %4$i)."
local wc_had_territory = _"%1$s had %2$3.0f%% of the land (%3$i of %4$i)."
local team_str = _"Team %i"

return {
   name = wc_name,
   description = wc_desc,
   func = function()
      local plrs = wl.Game().players

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

      -- Get all valueable fields of the map
      local fields = get_buildable_fields()

      -- variables to track the maximum 4 hours of gametime
      local remaining_max_time = 4 * 60 * 60 -- 4 hours

      -- These variables will be used once a player or team owns more than half
      -- of the map's area, to display the name of winning team/player.
      -- TODO(GunChleoc): We want to be able to list multiple winners in case of a draw.
      local last_winning_team = -1
      local last_winning_player = -1

      -- Used to keep track of when the winner changes
      local winning_players = {}
      local winning_teams = {}
      for idx,plr in ipairs(plrs) do
         winning_players[plr.number] = false
         winning_teams[plr.team] = false
      end

      -- Remaining time for victory by > 50% territory. Default value is also used to calculate whether to send a report to players.
      local remaining_time = 10 -- (dummy) -- time in secs, if == 0 -> victory

      local all_player_points = {}

      local function _calc_points()
         local points = {} -- tracking points of teams and players without teams
         local territory_was_kept = false

         all_player_points = count_owned_fields_for_all_players(fields, plrs)
         local ranked_players = rank_players(all_player_points, plrs)

         -- Check if we have a winner. The table was sorted, so we can simply grab the first entry.
         local winning_points = -1
         if ranked_players[1].points > ( #fields / 2 ) then
            winning_points = ranked_players[1].points
         end

         -- Calculate which team or player is the current winner, and whether the winner has changed
         for tidx, teaminfo in ipairs(ranked_players) do
            local is_winner = teaminfo.points == winning_points
            if teaminfo.team ~= 0 then
               points[#points + 1] = { team_str:format(teaminfo.team), teaminfo.points }
               if is_winner then
                  print("NOCOM Winner is team " .. teaminfo.team .. " with " .. teaminfo.points .. " points")
                  territory_was_kept = winning_teams[teaminfo.team]
                  winning_teams[teaminfo.team] = true
                  last_winning_team = teaminfo.team
                  last_winning_player = -1
               else
                  winning_teams[teaminfo.team] = false
               end
            end

            for pidx, playerinfo in ipairs(teaminfo.players) do
               if teaminfo.points ~= playerinfo.points then
                  winning_players[playerinfo.number] = false
               elseif is_winner and teaminfo.team == 0 then
                  print("NOCOM Winner is player " .. playerinfo.number .. " with " .. playerinfo.points .. " points")
                  territory_was_kept = winning_players[playerinfo.number]
                  winning_players[playerinfo.number] = true
                  last_winning_player = playerinfo.number
                  last_winning_team = -1
               else
                  winning_players[playerinfo.number] = false
               end
               if teaminfo.team == 0 and plrs[playerinfo.number] ~= nil then
                  points[#points + 1] = { plrs[playerinfo.number].name, playerinfo.points }
               end
            end
         end

         -- Set the remaining time according to whether the winner is still the same
         if territory_was_kept then
            -- Still the same winner
            remaining_time = remaining_time - 30
            print("NOCOM Territory was kept by " .. last_winning_team .. " - " .. last_winning_player .. ". Remaining time: " .. remaining_time)
         elseif winning_points == -1 then
            -- No winner. This value is used to calculate whether to send a report to players.
            remaining_time = 10
         else
            -- Winner changed
            remaining_time = 20 * 60 -- 20 minutes
            print("NOCOM NEW aqcuisition by " .. last_winning_team .. " - " .. last_winning_player .. ". Remaining time: " .. remaining_time)
         end
         return points
      end

      local function _percent(part, whole)
         return (part * 100) / whole
      end

      -- Helper function that returns a string containing the current
      -- land percentages of players/teams.
      local function _status(points, has_had)
         local msg = ""
         for i=1,#points do
            if (has_had == "has") then
               msg = msg ..
                  li(
                     (wc_has_territory):bformat(
                        points[i][1],
                        _percent(points[i][2], #fields),
                        points[i][2],
                        #fields))
            else
               msg = msg ..
                  li(
                     (wc_had_territory):bformat(
                        points[i][1],
                        _percent(points[i][2], #fields),
                        points[i][2],
                        #fields))
            end

         end
         return p(msg)
      end

      local function _send_state(points)
         set_textdomain("win_conditions")
         local winner_name = "Error"
         if last_winning_team >= 0 then
            winner_name = team_str:format(last_winning_team)
         elseif last_winning_player >= 0 then
            winner_name = plrs[last_winning_player].name
         end
         local msg1 = p(_"%s owns more than half of the map’s area."):format(winner_name)
         msg1 = msg1 .. p(ngettext("You’ve still got %i minute to prevent a victory.",
                   "You’ve still got %i minutes to prevent a victory.",
                   remaining_time // 60))
               :format(remaining_time // 60)
         msg1 = p(msg1)

         local msg2 = p(_"You own more than half of the map’s area.")
         msg2 = msg2 .. p(ngettext("Keep it for %i more minute to win the game.",
                   "Keep it for %i more minutes to win the game.",
                   remaining_time // 60))
               :format(remaining_time // 60)
         msg2 = p(msg2)

         for idx, pl in ipairs(plrs) do
            local msg = ""
            if remaining_time < remaining_max_time and
               (last_winning_team >= 0 or last_winning_player >= 0) then
               if last_winning_team == pl.team or last_winning_player == pl.number then
                  msg = msg .. msg2 .. vspace(8)
               else
                  msg = msg .. msg1 .. vspace(8)
               end
               -- TRANSLATORS: Refers to "You own more than half of the map’s area. Keep it for x more minute(s) to win the game."
               msg = msg .. p((ngettext("Otherwise the game will end in %i minute.",
                            "Otherwise the game will end in %i minutes.",
                            remaining_max_time // 60))
                  :format(remaining_max_time // 60))
            else
               msg = msg .. p((ngettext("The game will end in %i minute.",
                            "The game will end in %i minutes.",
                            remaining_max_time // 60))
                  :format(remaining_max_time // 60))
            end
            msg = msg .. vspace(8) .. game_status.body .. _status(points, "has")
            send_message(pl, game_status.title, msg, {popup = true})
         end
      end

      -- Start a new coroutine that checks for defeated players
      run(function()
         while remaining_time ~= 0 and remaining_max_time > 0 do
            sleep(5000)
            check_player_defeated(plrs, lost_game.title,
               lost_game.body, wc_descname, wc_version)
         end
      end)

      -- here is the main loop!!!
      while true do
         -- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
         sleep(30000)

         remaining_max_time = remaining_max_time - 30

         -- Check if a player or team is a candidate and update variables
         -- Returns the names and points for the teams and players without a team
         local points = _calc_points()

         -- Game is over, do stuff after loop
         if remaining_time <= 0 or remaining_max_time <= 0 then break end

         -- at the beginning send remaining max time message only each 30 minutes
         -- if only 30 minutes or less are left, send each 5 minutes
         -- also check if there is a candidate and we need to send an update
         if ((remaining_max_time < (30 * 60) and remaining_max_time % (5 * 60) == 0)
               or remaining_max_time % (30 * 60) == 0)
               or remaining_time % 300 == 0 then
            _send_state(points)
         end
      end

      local points = _calc_points()

      -- Game has ended
      for idx, pl in ipairs(plrs) do
         pl.see_all = 1

         maxpoints = points[1][2]
         local wonmsg = won_game_over.body
         wonmsg = wonmsg .. game_status.body
         local lostmsg = lost_game_over.body
         lostmsg = lostmsg .. game_status.body
         for i=1,#points do
            if points[i][1] == team_str:format(pl.team) or points[i][1] == pl.name then
               if points[i][2] >= maxpoints then
                  pl:send_message(won_game_over.title, wonmsg .. _status(points, "had"))
                  wl.game.report_result(pl, 1, make_extra_data(pl, wc_descname, wc_version, {score=all_player_points[pl.number]}))
               else
                  pl:send_message(lost_game_over.title, lostmsg .. _status(points, "had"))
                  wl.game.report_result(pl, 0, make_extra_data(pl, wc_descname, wc_version, {score=all_player_points[pl.number]}))
               end
            end
         end
      end
   end
}
