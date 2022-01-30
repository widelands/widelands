-- =======================================================================
--                   Territorial Lord Win condition
-- =======================================================================

include "scripting/coroutine.lua" -- for sleep
include "scripting/messages.lua"
include "scripting/table.lua"
include "scripting/win_conditions/win_condition_functions.lua"
include "scripting/win_conditions/territorial_functions.lua"

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Territorial Lord"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Territorial Lord")
local wc_version = 2
local wc_desc = _ (
   "Each player or team tries to obtain more than half of the map’s " ..
   "area. The winner will be the player or the team that is able to keep " ..
   "that area for at least 20 minutes. " ..
   "If the peaceful mode is selected, the game ends if one player has more " ..
   "land than any other player could gain."
)

local r = {
   name = wc_name,
   description = wc_desc,
   peaceful_mode_allowed = true,
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

      local function _send_state(show_popup)
         push_textdomain("win_conditions")

         for idx, player in ipairs(plrs) do
            local msg = ""
            if (territory_points.last_winning_team >= 0 or territory_points.last_winning_player >= 0) then
               if territory_points.last_winning_team == player.team or territory_points.last_winning_player == player.number then
                  msg = msg .. winning_status_header()
               else
                  msg = msg .. losing_status_header(plrs)
               end
            else
               msg = p(_("Currently no faction owns more than half of the map’s area."))
            end
            msg = msg .. vspace(8) .. game_status.body .. territory_status(fields, "has")
            send_to_inbox(player, game_status.title, msg, {popup = show_popup})
         end
         pop_textdomain()
      end

      -- Install statistics hook
      hooks.custom_statistic = statistics

      -- here is the main loop!!!
      while count_factions(plrs) > 1 and territory_points.remaining_time > 0 do
         -- Sleep 1 second
         sleep(1000)

         -- A player might have been defeated since the last calculation
         check_player_defeated(plrs, lost_game.title, lost_game.body)

         -- Check if a player or team is a candidate and update variables
         calculate_territory_points(fields, wl.Game().players)

         -- If there is a candidate, check whether we have to send an update
         if (territory_points.remaining_time % 300 == 0 and territory_points.remaining_time ~= 0) then
            local show_popup = false
            if territory_points.remaining_time % 600 == 0 then show_popup = true end
            _send_state(show_popup)
         end
      end

      -- Game has ended
      territory_game_over(fields, wl.Game().players, wc_descname, wc_version)
   end
}
pop_textdomain()
return r
