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

      -- Configure how long the winner has to hold on to the territory
      local time_to_keep_territory = 20 * 60 -- 20 minutes
      local remaining_time = time_to_keep_territory -- time in secs, if == 0 -> victory

      -- Get all valueable fields of the map
      local fields = get_buildable_fields()

      -- Remember current points for reports
      local all_player_points = {}
      local somebody_is_winning = false

      -- These variables will be used once a player or team owns more than half
      -- of the map's area
      local no_winner = {}
      no_winner["type"] = "none"
      local winner = no_winner

      local function _calc_points()
         local last_winner = winner
         all_player_points = count_owned_fields_for_all_players(fields, plrs)
         winner = find_winner(all_player_points, plrs)

         -- Check if we are (still) winning
         if winner["points"] > ( #fields / 2 ) then
            -- Calculate whether territory was kept or the winner changed. Make sure that the player still exists.
            if winner["type"] == last_winner["type"] and winner["winner"] == last_winner["winner"] then
               remaining_time = remaining_time - 30
            else
               remaining_time = time_to_keep_territory
            end
            last_winner = winner
            somebody_is_winning = true
         else
            -- Nobody is currently winning
            last_winner = no_winner
            remaining_time = time_to_keep_territory
            somebody_is_winning = false
         end
      end

      local function _send_state()
         local remaining_minutes = math.max(0, math.floor(remaining_time / 60))

         -- Player might have been defeated since the last time the winner was checked
         if remaining_minutes > 0 and plrs[winner["winner"]] then
            set_textdomain("win_conditions")
            local candidate = plrs[winner["winner"]].name
            if winner["type"] == "team" then
               candidate = (_"Team %i"):format(winner["winner"])
            end

            local msg1 = p(_"%s owns more than half of the map’s area."):format(candidate)
            msg1 = msg1 .. p(ngettext("You’ve still got %i minute to prevent a victory.",
                      "You’ve still got %i minutes to prevent a victory.",
                      remaining_minutes))
                  :format(remaining_minutes)

            local msg2 = p(_"You own more than half of the map’s area.")
            msg2 = msg2 .. p(ngettext("Keep it for %i more minute to win the game.",
                      "Keep it for %i more minutes to win the game.",
                      remaining_minutes))
                  :format(remaining_minutes)
            for idx, player in ipairs(plrs) do
               if winner["type"] == "team" and winner["winner"] == player.team
                  or winner["type"] == "player" and winner["winner"] == player.number then
                  send_message(player, game_status.title, msg2, {popup = true})
               else
                  send_message(player, game_status.title, msg1, {popup = true})
               end
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
            -- For formatting the winner's name
            local candidate = plrs[winner["winner"]].name
            if winner["type"] == "team" then
               candidate = (_"Team %i"):format(winner["winner"])
            end

            for idx, plr in ipairs(plrs) do
               plr.see_all = 1
               if winner["type"] == "team" and winner["winner"] == plr.team
                  or winner["type"] == "player" and winner["winner"] == plr.number then
                  plr:send_message(won_game_over.title, won_game_over.body .. p(_"You own more than half of the map’s area."))
                  wl.game.report_result(plr, 1, make_extra_data(plr, wc_descname, wc_version, {score=all_player_points[plr.number]}))
               else
                  plr:send_message(lost_game_over.title, lost_game_over.body .. p(_"%s owns more than half of the map’s area."):format(candidate))
                  wl.game.report_result(plr, 0, make_extra_data(plr, wc_descname, wc_version, {score=all_player_points[plr.number]}))
               end
            end
            break
         end

         -- If there is a candidate, check whether we have to send an update
         if somebody_is_winning and remaining_time >= 0 and remaining_time % 300 == 0 then
            _send_state()
         end
      end
   end
}
