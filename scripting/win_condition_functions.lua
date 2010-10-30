-- RST
-- win_condition_functions.lua
-- ---------------------------
--
-- This script contains functions that are shared by different win conditions.

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
--
--    :returns: :const:`nil`
function check_player_defeated(plrs, heading, msg)
   for idx,p in ipairs(plrs) do
      if p.defeated then
         p:send_message(heading, msg, { popup = true })
         p.see_all = 1
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
function broadcast(plrs, header, msg, goptions)
   local options = goptions or {}
   for idx, p in ipairs(plrs) do
      p:send_message(header, msg, options)
   end
end

