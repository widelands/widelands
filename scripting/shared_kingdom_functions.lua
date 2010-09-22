-- RST
-- shared_kingdom_functions.lua
-- ---------------------------
--
-- This script contains functions that are used for the shared kingdom feature.

-- =======================================================================
--                             PUBLIC FUNCTIONS
-- =======================================================================

-- RST
-- .. function:: actual_player(plr)
--
--    Checks whether the player plr has a partner and if yes, returns the
--    partner player, so the script can continue work on that player.
--
--    :arg plr: The player to be checked.
--
--    :returns: :const: Player (either plr or the partner player)
function actual_player(plr)
   if (plr.partner > 0) then
		return wl.Game().players[plr.partner]
   end
   return plr
end
