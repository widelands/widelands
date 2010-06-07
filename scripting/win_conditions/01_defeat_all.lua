-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "metaserver")

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
      for i=1,10 do
         if pcall(wl.game.Player, i) then
            plrs[#plrs+1] = wl.game.Player(i)
         end
      end

      local function count_factions()
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

      -- Iterate all players, if one is defeated, remove him
      -- from the list and send him a defeated message
      repeat
         sleep(5000)
         for idx,p in ipairs(plrs) do
            if p.defeated then
               _report_result(p, false)
               table.remove(plrs, idx)
               break
            end
         end
      until count_factions() <= 1

      -- Send congratulations to all remaining players
      for idx,p in ipairs(plrs) do
         _report_result(p, true)
      end

   end,
}
