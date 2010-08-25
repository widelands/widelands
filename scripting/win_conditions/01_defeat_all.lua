-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   description = _ "The tribe (or team) that can defeat all others wins!",
   func = function()
      -- Find all valid players
      local plrs = {}
      valid_players(plrs)

      -- Iterate all players, if one is defeated, remove him
      -- from the list, send him a defeated message and give him full vision
      repeat
         sleep(5000)
         check_player_defeated(plrs, _ "You lost!",
            _ "Sorry, you have lost this game!")
      until count_factions(plrs) <= 1

      -- Send congratulations to all remaining players
      for idx,p in ipairs(plrs) do
         p:send_message(
            _ "Congratulations!",
            _ "You have won this game!",
            {popup = true}
         )
      end

   end,
}
