-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "metaserver")
use("aux", "win_condition_functions")

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   description = _ "The tribe (or team) that can defeat all others wins!",
   func = function()

      -- =========================================                                                                                               
      -- Report the result back to the metaserver                                                                                                
      -- =========================================
      local function _report_result(p, has_won)
         if has_won then
            p:send_message(_"Congratulations!",
               _ "You have won this game!", 
               {popup = true}
            )
         else
            p:send_message(_ "You lost!",
               _ "Sorry, you have lost this game!",
               { popup = true }
            )
         end
         wl.game.report_result(p, has_won, 0, make_extra_data(p, "defeat_all", 1))
      end

      -- Find all valid players
      local plrs = {}
      valid_players(plrs)

      -- Iterate all players, if one is defeated, remove him
      -- from the list, send him a defeated message and give him full vision
      repeat
         sleep(5000)

         for idx,p in ipairs(plrs) do
            if p.defeated then
               _report_result(p, false)
               table.remove(plrs, idx)
               break
            end
         end

         check_player_defeated(plrs, _ "You lost!",
            _ "Sorry, you have lost this game!")
      until count_factions(plrs) <= 1

      -- Send congratulations to all remaining players
      for idx,p in ipairs(plrs) do
         _report_result(p, true)
      end

   end,
}
