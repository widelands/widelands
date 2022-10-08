-- RST
-- territorial_functions.lua
-- ---------------------------
--
-- This file contains common code for the "Territorial Lord" and "Territorial Time" win conditions.
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/win_conditions/territorial_functions.lua"
--

push_textdomain("win_conditions")

include "scripting/richtext.lua"
include "scripting/win_conditions/win_condition_functions.lua"
include "scripting/win_conditions/win_condition_texts.lua"

local team_str = _("Team %i")
local wc_has_territory = _("%1$s has %2$3.0f%% of the land (%3$i of %4$i).")
local wc_had_territory = _("%1$s had %2$3.0f%% of the land (%3$i of %4$i).")

local wc_stats = "Territory percentage"
-- This needs to be exactly like wc_stats, but localized, because wc_stats
-- will be used as the key to fetch the translation in C++
-- TRANSLATORS: subtext of the territorial statistics hook. Keep it short and consistent with the translation of the Win condition.
local wc_stats_i18n = _("Territory percentage")

-- Used by calculate_territory_points keep track of when the winner changes
local winning_players = {}
local winning_teams = {}

-- RST
-- .. data:: territory_points
--
--    This table contains information about the current points and winning status for all
--    players and teams:
--
--    .. code-block:: lua
--
--       territory_points = {
--          -- The currently winning team, if any. -1 means that no team is currently winning.
--          last_winning_team = -1,
--          -- The currently winning player, if any. -1 means that no player is currently winning.
--          last_winning_player = -1,
--          -- The name of the currently winning player, if any. Empty means that no player is currently winning.
--          last_winning_player_name = "",
--          -- Remaining time in secs for victory by > 50% territory. Default value is also used to calculate whether to send a report to players.
--          remaining_time = 10,
--          -- Points by player
--          all_player_points = {},
--          -- Points by rank, used to generate messages to the players
--          points = {}
--       }
--
territory_points = {
   -- TODO(GunChleoc): We want to be able to list multiple winners in case of a draw.
   last_winning_team = -1,
   last_winning_player = -1,
   -- We record the last winning player name here to prevent crashes with retrieving
   -- the player name when the player was just defeated a few ms ago
   last_winning_player_name = "",
   remaining_time = 1201,
   all_player_points = {},
   points = {}
}

-- variables for the territorial winconditions statsistics hook
fields = 0
statistics = {
      name = wc_stats,
      pic = "images/wui/stats/genstats_territorial_small.png",
      calculator = function(p)
         local pts = count_owned_valuable_fields_for_all_players(wl.Game().players)
         return (pts[p.number]*100//fields)
      end,
   }


-- RST
-- .. function:: calculate_territory_points(fields, players, wc_descname, wc_version)
--
--    First checks if a player was defeated, then fills the :data:`territory_points` table
--    with current data.
--
--    :arg fields: Number of all valuable fields
--    :type fields: :class:`integer`
--    :arg players: Table of all players
--    :type players: :class:`array` of :class:`~wl.game.Player`
--    :arg wc_descname: The win condition's descname
--    :type wc_descname: :class:`string`
--    :arg wc_version: The win condition's version
--    :type wc_version: :class:`integer`
--

function calculate_territory_points(fields, players)
   local points = {} -- tracking points of teams and players without teams
   local territory_was_kept = false

   territory_points.all_player_points = count_owned_valuable_fields_for_all_players(players)
   local ranked_players = rank_players(territory_points.all_player_points, players)

   -- Check if we have a winner. The table was sorted, so we can simply grab the first entry.
   local winning_points = -1
   -- Peaceful mode needs more land than any other can gain (lead points > remaining fields)
   local plrs = wl.Game().players
   if plrs[1]:is_attack_forbidden(plrs[2].number) then
      local remaining_points = fields
      for tidx, teaminfo in ipairs(ranked_players) do
         remaining_points = remaining_points - teaminfo.points
      end
      if (ranked_players[1].points - ranked_players[2].points) > remaining_points then
         winning_points = ranked_players[1].points
      end
   -- Without peaceful mode we need half the useful fields to win
   elseif ranked_players[1].points > ( fields / 2 ) then
      winning_points = ranked_players[1].points
   end

   -- Calculate which team or player is the current winner, and whether the winner has changed
   for tidx, teaminfo in ipairs(ranked_players) do
      local is_winner = teaminfo.points == winning_points
      if teaminfo.team ~= 0 then
         points[#points + 1] = { team_str:format(teaminfo.team), teaminfo.points }
         if is_winner then
            territory_was_kept = winning_teams[teaminfo.team] ~= nil
            winning_teams[teaminfo.team] = true
            territory_points.last_winning_team = teaminfo.team
            territory_points.last_winning_player = -1
            territory_points.last_winning_player_name = ""
         else
            winning_teams[teaminfo.team] = nil
         end
      end

      for pidx, playerinfo in ipairs(teaminfo.players) do
         if is_winner and teaminfo.team == 0 and teaminfo.points == playerinfo.points then
            territory_was_kept = winning_players[playerinfo.number] ~= nil
            winning_players[playerinfo.number] = true
            territory_points.last_winning_player = playerinfo.number
            territory_points.last_winning_player_name = playerinfo.name
            territory_points.last_winning_team = -1
         else
            winning_players[playerinfo.number] = nil
         end
         if teaminfo.team == 0 then
            points[#points + 1] = { playerinfo.name, playerinfo.points }
         end
      end
   end

   -- Set the remaining time according to whether the winner is still the same
   if territory_was_kept then
      -- Still the same winner
      territory_points.remaining_time = territory_points.remaining_time - 1
   elseif winning_points == -1 then
      -- No winner. We need to reset the information about last winner candidates
      territory_points.last_winning_player = -1
      territory_points.last_winning_player_name = -1
      territory_points.last_winning_team = -1
      -- This value is used to calculate whether to send a report to players.
      if territory_points.remaining_time == 1800 then
         territory_points.remaining_time = 1201
      elseif territory_points.remaining_time ~= 1201 then
         territory_points.remaining_time = 1800
      end

   else
      -- Winner changed
      territory_points.remaining_time = 20 * 60 -- 20 minutes
   end
   -- we are in peaceful mode and a player can't be stopped anymore
   if plrs[1]:is_attack_forbidden(plrs[2].number) and winning_points > 0 then
      territory_points.remaining_time = 0
   end

   territory_points.points = points
end

-- RST
-- .. function:: territory_status(fields, has_had)
--
--    Returns a string containing the current land percentages of players/teams
--    for messages to the players
--
--    :arg fields: Number of all valuable fields
--    :type fields: :class:`integer`
--    :arg has_had: Use "has" for an interim message, "had" for a game over message.
--    :type has_had: :class:`string`
--
--    :returns: a richtext-formatted string with information on current points for each player/team
--
function territory_status(fields, has_had)
   local function _percent(part, whole)
      return (part * 100) / whole
   end

   local msg = ""
   for i=1,#territory_points.points do
      if (has_had == "has") then
         msg = msg ..
            li(
               (wc_has_territory):bformat(
                  territory_points.points[i][1],
                  _percent(territory_points.points[i][2], fields),
                  territory_points.points[i][2],
                  fields))
      else
         msg = msg ..
            li(
               (wc_had_territory):bformat(
                  territory_points.points[i][1],
                  _percent(territory_points.points[i][2], fields),
                  territory_points.points[i][2],
                  fields))
      end

   end
   return p(msg)
end

-- RST
-- .. function:: winning_status_header()
--
--    Returns a string containing a status message header for a winning player
--
--    :returns: a richtext-formatted string with header information for a winning player
--
function winning_status_header()
   push_textdomain("win_conditions")
   local remaining_minutes = math.max(0, math.floor(territory_points.remaining_time / 60))

   local message = p(_("You own more than half of the map’s area."))
   message = message .. p(ngettext("Keep it for %i more minute to win the game.",
             "Keep it for %i more minutes to win the game.",
             remaining_minutes))
         :format(remaining_minutes)
   pop_textdomain()
   return message
end

-- RST
-- .. function:: losing_status_header(players)
--
--    Returns a string containing a status message header for a losing player
--
--    :arg players: Table of all players
--    :type players: :class:`array` of :class:`~wl.game.Player`
--
--    :returns: a richtext-formatted string with header information for a losing player
--
function losing_status_header(players)
   push_textdomain("win_conditions")
   local winner_name = "Error"
   if territory_points.last_winning_team >= 0 then
      winner_name = team_str:format(territory_points.last_winning_team)
   elseif territory_points.last_winning_player >= 0 then
      winner_name = territory_points.last_winning_player_name
   end
   local remaining_minutes = math.max(0, math.floor(territory_points.remaining_time / 60))

   local message = p(_("%s owns more than half of the map’s area.")):format(winner_name)
   message = message .. p(ngettext("You’ve still got %i minute to prevent a victory.",
             "You’ve still got %i minutes to prevent a victory.",
             remaining_minutes))
         :format(remaining_minutes)
   pop_textdomain()
   return message
end

-- RST
-- .. function:: territory_game_over(fields, players, wc_descname, wc_version)
--
--    Updates the territory points and sends game over reports
--
--    :arg fields: Number of all valuable fields
--    :type fields: :class:`integer`
--    :arg players: Table of all players
--    :type players: :class:`array` of :class:`~wl.game.Player`
--    :arg wc_descname: The win condition's descname
--    :type wc_descname: :class:`string`
--    :arg wc_version: The win condition's version
--    :type wc_version: :class:`integer`
--

function territory_game_over(fields, players, wc_descname, wc_version)
   calculate_territory_points(fields, players, wc_descname, wc_version)

   for idx, pl in ipairs(players) do
      if not pl.resigned then
         local wonmsg = won_game_over.body .. game_status.body
         local lostmsg = lost_game_over.body .. game_status.body
         for i=1,#territory_points.points do
            if territory_points.points[i][1] == team_str:format(pl.team) or territory_points.points[i][1] == pl.name then
               if territory_points.points[i][2] >= territory_points.points[1][2] then
                  pl:send_to_inbox(won_game_over.title, wonmsg .. territory_status(fields, "had"))
                  wl.game.report_result(pl, 1, make_extra_data(pl, wc_descname, wc_version, {score=territory_points.all_player_points[pl.number]}))
               else
                  pl:send_to_inbox(lost_game_over.title, lostmsg .. territory_status(fields, "had"))
                  wl.game.report_result(pl, 0, make_extra_data(pl, wc_descname, wc_version, {score=territory_points.all_player_points[pl.number]}))
               end
            end
         end
      end
   end
end

pop_textdomain()
