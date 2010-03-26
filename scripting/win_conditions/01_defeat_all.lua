-- =======================================================================
--                         Defeat all Win condition                         
-- =======================================================================

use("aux", "coroutine")

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   description = _ "The tribe that can defeat all others wins!",
   check_func = function() 
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
               p:send_message(_ "You lost!",
                  _ "Sorry, you have lost this game!",
                  { popup = true })
               table.remove(plrs, idx)
               break
            end
         end
      end

      -- Only one player left. This is the winner!
      local p = plrs[1]
      p:send_message(
         _ "Congratulations!",
         _ "You have won this game!", 
         {popup = true}
      )

   end,
}
