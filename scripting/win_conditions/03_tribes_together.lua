-- =======================================================================
--                         Defeat all Win condition                         
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "metaserver")

set_textdomain("win_conditions")

return {
   name = _ "Tribes together",
   description = _ (
"The three tribes play together vs each other. Vision is granted " .. 
"to all the players playing together"
), 

   func =
function() 

-- =========================================
-- Report the result back to the metaserver 
-- =========================================
local function _report_result(p, has_won)
   if has_won then
      p:send_message(
         _"Game over", _"You have won this game!",
         {popup = true}
      )
   else
      p:send_message(
      _"You've lost!", _"Your tribe is no more. You have lost this game",
         { popup = true }
      )
   end

   wl.game.report_result(p, has_won, 0,
      make_extra_data(p, "tribes_together", 1)
   )
end


local plrs_by_tribe = { barbarians = {}, empire = {}, atlanteans = {} }

-- Currently, sharing vision and playing together is not really possible
-- Therefore we simulate it in Lua: we check every field periodically by
-- whom it is seen. If it is seen by a friend, we also get vision. 
-- Note that we do not jump through loops here: Fog of war will no longer 
-- work like this. Also we are quite stupid and simply search the whole map, 
-- if we would concentrate in military buildings, this could be implemented
-- more efficiently.
function _share_vision(t) 
   local at_once = 10
   local i = 0
   for w = 0,wl.map.get_width()-1 do
      for h = 0,wl.map.get_height()-1 do
         local f = wl.map.Field(w,h)
         for idx,plr in ipairs(plrs_by_tribe[t]) do
            if plr:sees_field(f) then
               for idx1,oplr in ipairs(plrs_by_tribe[t]) do
                  if plr ~= oplr and not oplr:sees_field(f) then
                     oplr:reveal_fields{f}
                  end
               end
               break
            end
         end
      end
      i = i +1
      -- We do one at_once colums every half second. So we do not impact the
      -- game's performance as terribly
      if i == at_once then
         sleep(500)
         i = 0
      end
   end
end

function send_to_all(...)
   for tname, plrs in pairs(plrs_by_tribe) do
      for idx, plr in ipairs(plrs) do
         plr:send_message(...)
      end
   end
end

local ntribes = 0

function _check_winners()
   for tname, plrs in pairs(plrs_by_tribe) do
      -- Check if all plrs are defeated
      local ndefeated = 0
      for idx,plr in ipairs(plrs) do
         if plr.defeated == true then ndefeated = ndefeated + 1 end
      end
      if #plrs == ndefeated then 
         for idx,plr in ipairs(plrs) do
            _report_result(plr, false)
         end
         plrs_by_tribe[tname] = nil
         ntribes = ntribes - 1
         send_to_all(_"Tribe defeated!", _("The tribe %s is no more.")
            :format(tname))
      end
   end
   -- Game over?
   if ntribes == 1 then -- Inform the winners
      for tname, plrs in pairs(plrs_by_tribe) do
         for idx, plr in ipairs(plrs) do
            _report_result(plr, true)
         end
      end
   end
end

-- Find all valid players
local plrs = {}
for i=1,10 do
   if pcall(wl.game.Player, i) then
      local p = wl.game.Player(i)
      plrs[#plrs+1] = p
      local k = plrs_by_tribe[p.tribe]
      k[#k+1] = p
   end
end
-- Remove tribes that are not in this game
for tname, plrs in pairs(plrs_by_tribe) do
   if #plrs == 0 then 
      plrs_by_tribe[tname] = nil
   else
      ntribes = ntribes + 1
   end
end

while true do
   for tname, plrs in pairs(plrs_by_tribe) do _share_vision(tname) end
   if ntribes > 1 then _check_winners() end
end
end,
}
