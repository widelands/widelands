-- =======================================================================
--                         Defeat all Win condition                         
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "metaserver")

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   description = _ "The tribe that can defeat all others wins!",
   func = function() 

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

-- Iterate all players, if one is defeated, remove him
-- from the list and send him a defeated message
while #plrs > 1 do
   sleep(5000)
   for idx,p in ipairs(plrs) do
      if p.defeated then
         _report_result(p, false)
         table.remove(plrs, idx)
         break
      end
   end
end

-- Only one player left. This is the winner!
_report_result(plrs[1], true)

end,
}
