-- RST
-- win_condition_functions.lua
-- ---------------------------
--
-- This script contains functions that are shared by different win conditions.

use("aux", "metaserver")

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: check_player_defeated(plrs, heading, msg)
--
--    Checks whether one of the players in the list was defeated and if yes,
--    removes that player from the list and sends him/her a message.
--
--    :arg plrs:    List of Players to be checked
--    :arg heading: Heading of the message the defeated player will get
--    :arg msg:     Message the defeated player will get
--    :arg wc_name  Name of the win condition
--    :arg wc_ver   Version of the win condition
--
--    :returns: :const:`nil`
function check_player_defeated(plrs, heading, msg, wc_name, wc_ver)
   for idx,p in ipairs(plrs) do
      if p.defeated then
         p:send_message(heading, msg, { popup = true })
         p.see_all = 1
         wl.game.report_result(p, false, 0, make_extra_data(p, wc_name, wc_ver))
         table.remove(plrs, idx)
         break
      end
   end
end

-- RST
-- .. function:: valid_players(plrs)
--
--    Fills a list with all valid players currently in game.
--
--    :arg plrs: List the players will be saved to
--
--    :returns: :const:`nil`
function valid_players(plrs)
   for i=1,10 do
      if pcall(wl.game.Player, i) then
         plrs[#plrs+1] = wl.game.Player(i)
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
--    :method:`~wl.game.Player.send_message`. All parameters are passed
--    literally.
function broadcast(plrs, header, msg, goptions)
   local options = goptions or {}
   for idx, p in ipairs(plrs) do
      p:send_message(header, msg, options)
   end
end

function broadcast_win(plrs, header, msg, goptions, wc_name, wc_ver)
   local options = goptions or {}
   for idx, p in ipairs(plrs) do
       p:send_message(header, msg, options)
       wl.game.report_result(p, true, 0, make_extra_data(p, wc_name, wc_ver))
   end
end
