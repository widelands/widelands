-- =======================================================================
--                         Defeat all Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep

set_textdomain("win_conditions")

return {
   name = _ "Autocrat",
   description = _ "The tribe (or team) that can defeat all others wins!",
   func = function()
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
      -- from the list, send him a defeated message and give him full vision
      repeat
         sleep(5000)
         for idx,p in ipairs(plrs) do
            if p.defeated then
               p:send_message(_ "You lost!",
                  _ "Sorry, you have lost this game!",
                  { popup = true })
               p.see_all = 1
               table.remove(plrs, idx)
               break
            end
         end
      until count_factions() <= 1

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
