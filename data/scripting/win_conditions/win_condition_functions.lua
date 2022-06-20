include "scripting/richtext.lua"
include "scripting/messages.lua"
include "scripting/table.lua"

-- RST
-- win_condition_functions.lua
-- ---------------------------
--
-- This script contains functions that are shared by different win conditions.
--
-- To make these functions available include this file at the beginning
-- of a script via:
--
-- .. code-block:: lua
--
--    include "scripting/win_conditions/win_condition_functions.lua"
--

-- RST
-- .. function:: make_extra_data(plr, name, version[, extra])
--
--    Constructs a string containing information about the win condition.
--    This can e.g be used to inform the metaserver about it.
--
--    :arg plr: Player to calculate extra data for
--    :type plr: :class:`~wl.game.Player`
--    :arg name: Name of the win-condition
--    :type name: :class:`string`
--    :arg version: Version the win-condition
--    :type version: :class:`integer`
--    :arg extra: List of other extra arguments that should be passed
--                to the server. They will also be incorporated into the
--                extra string.
--    :type extra: :class:`array`
--
--    :returns: The extra string that can be passed on
--

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

-- don't declare players defeated instantly with Discovery starting condition
win_conditions__initially_without_warehouse = {}

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: check_player_defeated(plrs, heading, msg, wc_name, wc_ver)
--
--    Checks whether one of the players in the list was defeated and if yes,
--    removes that player from the list and sends him/her a message.
--
--    :arg plrs: List of :class:`players <wl.game.Player>` to be checked
--    :type plrs: :class:`array`
--    :arg heading: Heading of the message the defeated player will get
--    :type heading: :class:`string`
--    :arg msg:     Message the defeated player will get
--    :type msg: :class:`string`
--    :arg wc_name: Name of the win condition. If not nil, :meth:`wl.game.report_result`
--                  will be called.
--    :type wc_name: :class:`string`
--    :arg wc_ver:  Version of the win condition
--    :type wc_ver: :class:`integer`
--
--    :returns: :const:`nil`

function check_player_defeated(plrs, heading, msg, wc_name, wc_ver)
   for idx,p in ipairs(plrs) do
      if win_conditions__initially_without_warehouse[idx] == nil then
         win_conditions__initially_without_warehouse[idx] = p.defeated
      elseif (p.defeated and not win_conditions__initially_without_warehouse[idx]) or p.resigned then
         if not p.resigned then p:send_to_inbox(heading, msg) end
         local fields_to_destroy = {}
         -- first sink all ships a player still has
         for idx,s in ipairs(array_combine(p:get_ships())) do
            s:destroy()
         end
         -- now collect the warehouses/ports/milsites including constructionsites a player has
         for name,allowed in ordered_pairs(p.allowed_buildings) do
            for i,site in ipairs(p:get_constructionsites(name)) do
               table.insert(fields_to_destroy, site.fields[1])
            end
            for i,site in ipairs(p:get_buildings(name)) do
               if site.descr.type_name == "militarysite" or site.descr.type_name == "warehouse" then
                  table.insert(fields_to_destroy, site.fields[1])
               end
            end
         end
         -- destroy the collected sites
         for idx,f in ipairs(fields_to_destroy) do
            if f.immovable then
               f.immovable:destroy()
               -- add some delay to the destruction for dramaturgical reason
               sleep(400)
            end
         end
         p.see_all = 1
         if (wc_name and wc_ver) and not p.resigned then
            wl.game.report_result(p, 0, make_extra_data(p, wc_name, wc_ver))
         end
         table.remove(plrs, idx)
         break
      elseif not p.defeated then
         win_conditions__initially_without_warehouse[idx] = false
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
--    :arg plrs: List of :class:`players <wl.game.Player>`
--    :type plrs: :class:`array`
--
--    :returns: The number of factions left in game
--

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
--    Broadcast a message to all players using
--    :meth:`send_to_inbox`. All parameters are passed
--    literally.

function broadcast(plrs, header, msg, goptions)
   local options = goptions or {}
   for idx, p in ipairs(plrs) do
      send_to_inbox(p, header, msg, options)
   end
end

function broadcast_win(plrs, header, msg, goptions, wc_name, wc_ver, gextra)
   local options = goptions or {}
   local extra = gextra or {}
   for idx, p in ipairs(plrs) do
       p:send_to_inbox(header, msg, options)
       wl.game.report_result(p, 1, make_extra_data(p, wc_name, wc_ver, extra))
   end
end

function broadcast_lost(plrs, header, msg, goptions, wc_name, wc_ver, gextra)
   local options = goptions or {}
   local extra = gextra or {}
   for idx, p in ipairs(plrs) do
       p:send_to_inbox(header, msg, options)
       wl.game.report_result(p, 0, make_extra_data(p, wc_name, wc_ver, extra))
   end
end


-- RST
-- .. function:: broadcast_objective(header, msg, body)
--
--    Broadcast an :class:`~wl.game.Objective` to all players. Technically,
--    it is assigned to player1, because all players will see all objectives.
--
--    :arg name:  A unique name for the objective
--    :type name: :class:`string`
--    :arg title: The title to be displayed for the objective
--    :type title: :class:`string`
--    :arg body: The content text to be displayed for the objective
--    :type body: :class:`string`

function broadcast_objective(name, title, body)
   local plrs = wl.Game().players
   plrs[1]:add_objective(name, title, body)
end


-- RST
-- .. function:: count_owned_valuable_fields_for_all_players(players[, attribute])
--
--    Counts all owned fields for each player.
--
--    :arg players: Table of all :class:`players <wl.game.Player>`
--    :type players: :class:`array` of :class:`wl.game.Player`
--    :arg attribute: If this is set, only count fields that have an immovable
--                    with this attribute.
--    :type attribute: :class:`string`
--
--    :returns: A table with ``playernumber=count_of_owned_fields``  entries
--

function count_owned_valuable_fields_for_all_players(players, attribute)
   attribute = attribute or ""

   local owned_fields = {}

   -- Get number of currently owned valuable fields per player.
   -- This table can contain defeated players.
   local all_plrpoints = wl.Game().map:count_owned_valuable_fields(attribute)

   -- Insert points for all players who are still in the game, and 0 points for defeated players.
   for idx,plr in ipairs(players) do
      if (plr.defeated) or (all_plrpoints[plr.number] == nil) then
         owned_fields[plr.number] = 0
      else
         owned_fields[plr.number] = all_plrpoints[plr.number]
      end
   end
   return owned_fields
end



-- RST
-- .. function:: rank_players(all_player_points, plrs)
--
--    Rank the players and teams according to the highest points
--
--    :arg all_player_points: A table of ``playernumber=points`` entries for
--                            all players.
--    :type all_player_points: :class:`array`
--    :arg plrs: A table of all :class:`wl.game.Player` objects
--    :type plrs: :class:`array`
--
--    :returns: A table with ranked player and team points, sorted by points
--              descending. Example:
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
               { number = player.number, name = player.name, points = player_points }
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
            table.insert(team_table.players, { number = player.number, name = player.name, points = all_player_points[player.number] })
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

-- RST
-- .. function:: format_remaining_time(remaining_time)
--
--    Return a localized message that contains the remaining game time
--    to be used when sending status meassages.
--
--    :arg remaining_time: The remaining game time in minutes.
--    :type remaining_time: :class:`integer`
--

function format_remaining_time(remaining_time)
   local h = 0
   local m = 60
   local time = ""
   push_textdomain("win_conditions")

   if (remaining_time ~= 60) then
      h = math.floor(remaining_time / 60)
      m = remaining_time % 60
   end

   if ((h > 0) and (m > 0)) then
      -- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
      time = (ngettext("%i minute", "%i minutes", h, m)):bformat(m)
      -- TRANSLATORS: Context: 'The game will end in 2 hours and 30 minutes.'
      time = (ngettext("%1% hour and %2%", "%1% hours and %2%", h, m)):bformat(h, time)
   elseif m > 0 then
      -- TRANSLATORS: Context: 'The game will end in 30 minutes.'
      time = (ngettext("%i minute", "%i minutes", m)):bformat(m)
   else
      -- TRANSLATORS: Context: 'The game will end in 2 hours.'
      time = (ngettext("%1% hour", "%1% hours", h)):bformat(h)
   end
   -- TRANSLATORS: Context: 'The game will end in (2 hours and) 30 minutes.'
   local result = p(_("The game will end in %s.")):bformat(time)
   pop_textdomain()
   return result
end

-- RST
-- .. function:: notification_remaining_time(max_time, remaining_time)
--
--    Calculate the remaining game time for notifications.
--    Should only be called within a coroutine, because the routine gets blocked.
--    Returns the remaining time and whether the notification should popup.
--
--    To be used when sending status messages.
--    Status messages are to be sent every 30 minutes and every 5 during the last 30 minutes,
--    the message window pops up ever hour, 30, 20 & 10 minutes before the game ends.
--
--    :arg max_time:    The time maximum game time in minutes
--    :type max_time: :class:`integer`
--    :arg remaining_time: The remaining time until game ends. On first call
--                         this is equal to **max_time**.
--    :type remaining_time: :class:`integer`
--
--    :returns: The remaining_time and :class:`true` if the end of the
--              predefined periods are reached.
--

function notification_remaining_time(max_time, remaining_time)
   local show_popup = false
   if (wl.Game().time == 100) then
     -- Show status at start of the game
     -- Execution time is set in logic/game.cc::init_newgame
     wake_me(wl.Game().time + 1)
   elseif (wl.Game().time < ((max_time - 30) * 60 * 1000)) then --
      wake_me(wl.Game().time + (30 * 60 * 1000)) -- 30 minutes
      remaining_time = remaining_time - 30
      if (remaining_time % 60 == 0) or (remaining_time == 30) then show_popup = true end
   else
      wake_me(wl.Game().time + (300 * 1000)) --5 Minutes
      remaining_time = remaining_time - 5
      if ((remaining_time ~= 0) and (remaining_time % 10 == 0)) then show_popup = true end
   end
   return remaining_time, show_popup
end
