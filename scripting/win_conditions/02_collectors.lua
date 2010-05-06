-- =======================================================================
--                         Collectors Win condition
-- =======================================================================

use("aux", "coroutine") -- for sleep
use("aux", "table")
use("aux", "metaserver")

set_textdomain("win_conditions")

return {
   name = _ "Collectors",
   description = _ (
"You get points for wares in your HQ, the one with the highest number " .. 
"of wares at the end of 4 hours wins the game"
),
   func = function() 

local total_time = 4 * 60 -- 4 hours

-- =========================================
-- Report the state back to the Meta Server 
-- =========================================
local function _report_result(p, points, has_won)
   if has_won then
      p:send_message(_"You won!", _"You are the winner!",
         {popup = true}
      )
   else
      p:send_message(
         _"You lost", _"You've lost this game!",
         { popup = true }
      )
   end

   wl.game.report_result(p, has_won, points,
      make_extra_data(p, "collectors", 1, {
         total_time = total_time
      }
   ))
end

-- =========================================
-- Calculate the current points information 
-- =========================================
function _calc_points(p)
   local bs = array_combine(
      p:get_buildings("headquarters"), p:get_buildings("warehouse")
   )
   local rv = 0
   for idx,b in ipairs(bs) do
      for wname, count in pairs(b:get_wares("all")) do
         rv = rv + count
      end
   end
   return rv
end

-- =========================================================
-- Send Information to the players about the current points 
-- =========================================================
function _send_state(plrs, remaining_time)
   local msg = _("The game will end in %i minutes."):format(remaining_time)
   msg = msg .. "\n\n"

   for idx, p in ipairs(plrs) do
      msg = msg .. _("The player %i has currently %i points\n"):
         format(p.number, _calc_points(p))
   end

   for idx, p in ipairs(plrs) do
      p:send_message(_ "Status", msg)
   end
end

sleep(1000)

-- Find all valid players
local plrs = {}
for i=1,10 do
   if pcall(wl.game.Player, i) then
      plrs[#plrs+1] = wl.game.Player(i)
   end
end

local remaining_time = total_time
while true do
   _send_state(plrs, remaining_time)

   sleep(math.min(10,remaining_time) * 60 * 1000) -- Sleep a while
   remaining_time = remaining_time - 10

   -- Game ended?
   if remaining_time <= 0 then
      local points = {}
      for idx,plr in ipairs(plrs) do
         points[#points + 1] = { plr, _calc_points(plr) }
      end
      table.sort(points, function(a,b) return a[2] < b[2] end)
      for i=1,#points-1 do
         _report_result(points[i][1], points[i][2], false)
      end
      _report_result(points[#points][1], points[#points][2], true)
      break
   end
end
   end
}
