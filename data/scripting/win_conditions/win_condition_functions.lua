include "scripting/richtext.lua"
include "scripting/messages.lua"

-- RST
-- win_condition_functions.lua
-- ---------------------------
--
-- This script contains functions that are shared by different win conditions.

-- RST
-- .. function:: make_extra_data(plr, name, version[, extra])
--
--    Constructs a string containing information about the win condition.
--    this can e.g be used to inform the  meta server about it.
--
--    :arg plr: Player to calculate extra data for
--    :type plr: :class:`~wl.game.Player`
--    :arg name: Name of the win-condition
--    :type name: :class:`string`
--    :arg version: Version the win-condition
--    :type version: :class:`integer`
--    :arg extra: list of other extra arguments that should be passed
--       to the server. They will also be incorporated into the extra string.
--    :type extra: :class:`array`
--
--    :returns: the extra string that can be passed on
function make_extra_data(plr, name, version, extra)
   extra = extra or {}
   local rv = {
      "win_condition=" .. name,
      "win_condition_version=" .. version,
      "tribe=" .. plr.tribe_name,
   }

   for k,v in pairs(extra) do
      rv[#rv+1] = ("%s=%s"):format(k,v)
   end

   return table.concat(rv, ";")
end

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: check_player_defeated(plrs, heading, msg, wc_name, wc_ver)
--
--    Checks whether one of the players in the list was defeated and if yes,
--    removes that player from the list and sends him/her a message.
--
--    :arg plrs:    List of Players to be checked
--    :arg heading: Heading of the message the defeated player will get
--    :arg msg:     Message the defeated player will get
--    :arg wc_name: Name of the win condition. If not nil, meth:`wl.game.Game.report_result`
--       will be called.
--    :arg wc_ver:  Version of the win condition
--
--    :returns: :const:`nil`
function check_player_defeated(plrs, heading, msg, wc_name, wc_ver)
   for idx,p in ipairs(plrs) do
      if p.defeated then
         p:send_message(heading, msg)
         p.see_all = 1
         if (wc_name and wc_ver) then
            wl.game.report_result(p, 0, make_extra_data(p, wc_name, wc_ver))
         end
         table.remove(plrs, idx)
         break
      end
   end
end

-- RST
-- .. function:: count_factions(plrs)
--
--    Calculates and returns the number of factions that are still involved in
--    the running game.
--    A faction is a team or an unteamed player.
--
--    :arg plrs: List the players will be saved to
--
--    :returns: the number of factions left in game
function count_factions(plrs)
   local factions = 0
   local teams = {}
   for idx,p in ipairs(plrs) do
      local team = p.team
      if team == 0 then
         factions = factions + 1
      else
         if not teams[team] then
            teams[team] = true
            factions = factions + 1
         end
      end
   end
   return factions
end

-- RST
-- .. function:: broadcast(plrs, header, msg[, options])
--
--    broadcast a message to all players using
--    :meth:`~wl.game.Player.send_message`. All parameters are passed
--    literally.
--    Message is delayed while player is in road building mode.
function broadcast(plrs, header, msg, goptions)
   local options = goptions or {}
   for idx, p in ipairs(plrs) do
      send_message(p, header, msg, options)
   end
end

function broadcast_win(plrs, header, msg, goptions, wc_name, wc_ver, gextra)
   local options = goptions or {}
   local extra = gextra or {}
   for idx, p in ipairs(plrs) do
       p:send_message(header, msg, options)
       wl.game.report_result(p, 1, make_extra_data(p, wc_name, wc_ver, extra))
   end
end

function broadcast_lost(plrs, header, msg, goptions, wc_name, wc_ver, gextra)
   local options = goptions or {}
   local extra = gextra or {}
   for idx, p in ipairs(plrs) do
       p:send_message(header, msg, options)
       wl.game.report_result(p, 0, make_extra_data(p, wc_name, wc_ver, extra))
   end
end


-- RST
-- .. function:: broadcast_objective(header, msg, body)
--
--    broadcast an objective to all players
--    technically, it is assigned to player1, because all players will see all objectives
--
--    :arg name:    A unique name for the objective
--    :arg title:   The title to be displayed for the objective
--    :arg body:    The content text to be displayed for the objective
function broadcast_objective(name, title, body)
   local plrs = wl.Game().players
   plrs[1]:add_objective(name, title, body)
end


-- RST
-- .. function:: rank_players(all_player_points, plrs)
--
--    Rank the players and teams according to the highest points
--
--    :arg all_player_points:    A table of ``playernumber = points`` entries for all players
--    :arg plrs:                 A table of all Player objects
--
--    :returns: A table with ranked player and team points, sorted by points descending. Example:
--
--    .. code-block:: lua
--
--       {
--          -- A player without team
--          {
--             team = 0,
--             points = 1000,
--             players = {
--                { "number" = 5, "points" = 1000 }
--             }
--          },
--          -- This team has a draw with player 5
--          {
--             team = 1,
--             points = 1000,
--             players = {
--                { "number" = 2, "points" = 500 }
--                { "number" = 3, "points" = 400 }
--                { "number" = 4, "points" = 100 }
--          },
--          -- Another player without team
--          {
--             team = 0,
--             points = 800,
--             players = {
--                { "number" = 1, "points" = 800 }
--             }
--          },
--       }
--
function rank_players(all_player_points, plrs)
   local ranked_players_and_teams = {}
   local team_points = {}

   -- Add points for players without teams and calculate team points
   for idx, player in ipairs(plrs) do
      local player_points = all_player_points[player.number]
      local team = player.team
      if team == 0 then
         -- Player without team - add it directly
         local team_table = {
            team = 0,
            points = player_points,
            players = {
               { number = player.number, points = player_points }
            }
         }
         table.insert(ranked_players_and_teams, team_table)
      else
         -- Team player - add to team points
         if not team_points[team] then
            team_points[team] = 0
         end
         team_points[team] = team_points[team] + player_points
      end
   end

   -- Add points for teams and their players
   for team, points in pairs(team_points) do
      local team_table = {
         team = team,
         points = points,
         players = {}
      }
      for idx, player in ipairs(plrs) do
         if player.team == team then
            table.insert(team_table.players, { number = player.number, points = all_player_points[player.number] })
         end
      end
      table.insert(ranked_players_and_teams, team_table)
   end

   -- Sort the players by points descending
   for ids, team in pairs(ranked_players_and_teams) do
      table.sort(team.players, function(a,b) return a["points"] > b["points"] end)
   end

   -- Sort the teams by points descending
   table.sort(ranked_players_and_teams, function(a,b) return a["points"] > b["points"] end)
   return ranked_players_and_teams
end
