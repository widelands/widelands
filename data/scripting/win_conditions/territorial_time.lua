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

push_textdomain("win_conditions")

include "scripting/win_conditions/win_condition_texts.lua"

local wc_name = "Territorial Time"
-- This needs to be exactly like wc_name, but localized, because wc_name
-- will be used as the key to fetch the translation in C++
local wc_descname = _("Territorial Time")
local wc_version = 1
local wc_desc_placeholder = _(
   "Each player or team tries to obtain more than half of the map’s " ..
   "area. The winner will be the player or the team that is able to keep " ..
   "that area for at least 20 minutes, or the one with the most territory " ..
   "after %s, whichever comes first. " ..
   "If the peaceful mode is selected, the game ends if one player has more " ..
   "land than any other player could gain."
)
-- TRANSLATORS: Will be inserted into "The winner will be the player […] with the most territory after %s"
local wc_desc = wc_desc_placeholder:bformat(_("the configured time limit"))

local r = {
   name = wc_name,
   description = wc_desc,
   configurable_time = true,
   peaceful_mode_allowed = true,
   init = function()
      fields = wl.Game().map:count_conquerable_fields()
   end,
   func = function()
      local game = wl.Game()
      local plrs = wl.Game().players

      -- variables to track the maximum 4 hours of gametime
      local max_time = game.win_condition_duration

      -- set the objective with the game type for all players
      broadcast_objective("win_condition", wc_descname, wc_desc_placeholder:bformat(format_remaining_raw_time(max_time)))

      local function _send_state(remaining_time, plrs, show_popup)
         push_textdomain("win_conditions")

         local remaining_time_minutes = remaining_time // 60
         for idx, player in ipairs(plrs) do
            local msg = ""
            if territory_points.remaining_time < remaining_time and
               (territory_points.last_winning_team >= 0 or territory_points.last_winning_player >= 0) then
               if territory_points.last_winning_team == player.team or territory_points.last_winning_player == player.number then
                  msg = msg .. winning_status_header()
               else
                  msg = msg .. losing_status_header(plrs)
               end
            elseif remaining_time <= 1200 then
               territory_points.remaining_time = remaining_time
               msg = msg .. format_remaining_time(remaining_time_minutes)
            else
               msg = msg .. format_remaining_time(remaining_time_minutes)
            end
            msg = msg .. vspace(8) .. game_status.body .. territory_status(fields, "has")
            send_to_inbox(player, game_status.title, msg, {popup = show_popup})
         end
         pop_textdomain()
      end

      -- Start a new coroutine that triggers status notifications.
      run(function()
         local remaining_time = max_time
         local msg = ""
         while game.time <= ((max_time - 5) * 60 * 1000) and count_factions(plrs) > 1 and territory_points.remaining_time > 0 do
            remaining_time, show_popup = notification_remaining_time(max_time, remaining_time)
            if territory_points.remaining_time == 1201 then
               msg = format_remaining_time(remaining_time) .. vspace(8) .. game_status.body .. territory_status(fields, "has")
               broadcast(plrs, game_status.title, msg, {popup = show_popup})
            end
         end
      end)

      -- Install statistics hook
      hooks.custom_statistic = statistics

      -- here is the main loop!!!
      while game.time < (max_time * 60 * 1000) and count_factions(plrs) > 1 and territory_points.remaining_time > 0 do
         -- Sleep 1 second
         sleep(1000)

         -- A player might have been defeated since the last calculation
         check_player_defeated(plrs, lost_game.title, lost_game.body)

         -- Check if a player or team is a candidate and update variables
         -- Returns the names and points for the teams and players without a team
         calculate_territory_points(fields, plrs)

         -- check if there is a candidate and we need to send an update
         if (territory_points.remaining_time % 300 == 0 and territory_points.remaining_time ~= 0) then
            local remaining_time = (max_time * 60 * 1000 - game.time) // 1000
            local show_popup = false

            if territory_points.remaining_time % 600 == 0 then show_popup = true end
            _send_state(remaining_time, plrs, show_popup)

         end
      end

      -- Game has ended
      territory_game_over(fields, wl.Game().players, wc_descname, wc_version)
   end
}
pop_textdomain()
return r
