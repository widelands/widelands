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

      local function _send_state()
         set_textdomain("win_conditions")

         local remaining_max_minutes = remaining_max_time // 60
         for idx, player in ipairs(plrs) do
            local msg = ""
            if territory_points.remaining_time < remaining_max_time and
               (territory_points.last_winning_team >= 0 or territory_points.last_winning_player >= 0) then
               if territory_points.last_winning_team == player.team or territory_points.last_winning_player == player.number then
                  msg = msg .. winning_status_header() .. vspace(8)
               else
                  msg = msg .. losing_status_header(plrs) .. vspace(8)
               end
               -- TRANSLATORS: Refers to "You own more than half of the map’s area. Keep it for x more minute(s) to win the game."
               msg = msg .. p((ngettext("Otherwise the game will end in %i minute.",
                            "Otherwise the game will end in %i minutes.",
                            remaining_max_minutes))
                  :format(remaining_max_minutes))
            else
               msg = msg .. p((ngettext("The game will end in %i minute.",
                            "The game will end in %i minutes.",
                            remaining_max_minutes))
                  :format(remaining_max_minutes))
            end
            msg = msg .. vspace(8) .. game_status.body .. territory_status(fields, "has")
            broadcast(plrs, game_status.title, msg, {popup = true})
         end
      end

      -- here is the main loop!!!
      while true do
         -- Sleep 30 seconds == STATISTICS_SAMPLE_TIME
         sleep(30000)

         remaining_max_time = remaining_max_time - 30
         -- A player might have been defeated since the last calculation
         check_player_defeated(plrs, lost_game.title, lost_game.body)
         -- Check if a player or team is a candidate and update variables
         -- Returns the names and points for the teams and players without a team
         calculate_territory_points(fields, wl.Game().players)

         -- Game is over, do stuff after loop
         if territory_points.remaining_time <= 0 or remaining_max_time <= 0 or count_factions(plrs) <= 1 then break end

         -- at the beginning send remaining max time message only each 30 minutes
         -- if only 30 minutes or less are left, send each 5 minutes
         -- also check if there is a candidate and we need to send an update
         if ((remaining_max_time < (30 * 60) and remaining_max_time % (5 * 60) == 0)
               or remaining_max_time % (30 * 60) == 0)
               or territory_points.remaining_time % 300 == 0 then
            _send_state()
         end
      end

      -- Game has ended
      territory_game_over(fields, wl.Game().players, wc_descname, wc_version)
   end
}
