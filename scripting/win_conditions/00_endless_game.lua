-- =======================================================================
--                         An endless game without rules
-- =======================================================================

use("aux", "coroutine") -- for sleep

set_textdomain("win_conditions")

return {
   name = _ "Endless Game",
   description = _"An endless game without rules",
   -- This function just cares about players that were defeated and gives them
   -- full vision
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
               p:send_message(_ "You are defeated!",
                  _ "You have nothing to command left. If you want, you may continue as spectator.",
                  { popup = true })
               p.see_all = 1
               table.remove(plrs, idx)
               break
            end
         end
      until count_factions() < 1
      
   end
}
