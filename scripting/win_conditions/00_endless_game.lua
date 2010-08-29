-- =======================================================================
--                         An endless game without rules
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

return {
   name = _ "Endless Game",
   description = _"An endless game without rules",
   -- This function just cares about players that were defeated and gives them
   -- full vision
   func = function()
      -- Find all valid players
      local plrs = {}
      valid_players(plrs)

      -- Iterate all players, if one is defeated, remove him
      -- from the list, send him a defeated message and give him full vision
      repeat
         sleep(5000)
         check_player_defeated(plrs, _ "You are defeated!",
            _ "You have nothing to command left. If you want, you may " ..
              "continue as spectator.")
      until count_factions(plrs) < 1
      
   end
}
