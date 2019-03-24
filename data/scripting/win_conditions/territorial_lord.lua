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

local fields = 0

return {
   name = wc_name,
   description = wc_desc,
   init = function()
      fields = wl.Game().map:count_conquerable_fields()
   end,
   func = function()
      local plrs = wl.Game().players

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc)

      -- Configure how long the winner has to hold on to the territory
      local time_to_keep_territory = 20 * 60 -- 20 minutes
      -- time in secs, if == 0 -> victory
      territory_points.remaining_time = time_to_keep_territory

      local function _send_state()
         set_textdomain("win_conditions")

         for idx, player in ipairs(plrs) do
            local msg = ""
            if territory_points.last_winning_team == player.team or territory_points.last_winning_player == player.number then
               msg = msg .. winning_status_header() .. vspace(8)
            else
               msg = msg .. losing_status_header(plrs) .. vspace(8)
            end
            msg = msg .. vspace(8) .. game_status.body .. territory_status(fields, "has")
            player:send_message(game_status.title, msg, {popup = true})
         end
      end

      -- here is the main loop!!!
      while true do
         -- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
         sleep(30000)

         -- A player might have been defeated since the last calculation
         check_player_defeated(plrs, lost_game.title, lost_game.body)

         -- Check if a player or team is a candidate and update variables
         calculate_territory_points(fields, wl.Game().players)

         -- Do this stuff, if the game is over
         if territory_points.remaining_time == 0 or count_factions(plrs) <= 1 then
            territory_game_over(fields, wl.Game().players, wc_descname, wc_version)
            break
         end

         -- If there is a candidate, check whether we have to send an update
         if (territory_points.last_winning_team >= 0 or territory_points.last_winning_player >= 0) and territory_points.remaining_time >= 0 and territory_points.remaining_time % 300 == 0 then
            _send_state()
         end
      end
   end
}
