use("core", "coroutine")

-- RST
-- smooth_move.lua
-- ---------------
--
-- This script contains one function to make a nice move transition from
-- the players current viewpoint to another place on the map.

-- =======================================================================
--                             PRIVATE FUNCTIONS                            
-- =======================================================================
function _fm_move(x,y, g_plr, g_T)
   local f = wl.map.Field(x,y)
   local plr = g_plr or wl.game.Player(1)
   local T = g_T or 1000

   local start = { x = plr.viewpoint_x, y = plr.viewpoint_y }
   local dest = { x = f.viewpoint_x, y = f.viewpoint_y }
   local delta = { x = dest.x - start.x, y = dest.y - start.y }

   -- Accelerate for 25% of time, then move constant, decelerate 25% of time
   local t0 = 0.25*T
   local t1 = 0.75*T
   local a = { x = delta.x/(t0*t1), y = delta.y/(t0*t1) }

   function s(t)
      if t < t0 then
         return {
            x = start.x + 0.5*a.x*t*t,
            y = start.y + 0.5*a.y*t*t
         }
      elseif t < t1 then
         return {
            x = start.x + 0.5*a.x*t0*t0 + (a.x*t0)*(t-t0),
            y = start.y + 0.5*a.y*t0*t0 + (a.y*t0)*(t-t0)
         }
      else 
         return {
            x = start.x + 0.5*a.x*t0*t0 + 
               (a.x*t0)*(t-t0) - 0.5*a.x*(t-t1)*(t-t1),
            y = start.y + 0.5*a.y*t0*t0 + 
               (a.y*t0)*(t-t0) - 0.5*a.y*(t-t1)*(t-t1)
         }
      end
   end

   local t = 0
   local dt = 20
   while t < T do
      cpos = s(t)
      plr.viewpoint_x = cpos.x
      plr.viewpoint_y = cpos.y
      t = t + dt

      sleep(dt)
   end
   -- Correct some numerical instabilities
   plr.viewpoint_x = dest.x
   plr.viewpoint_y = dest.y
end

-- =======================================================================
--                             PUBLIC FUNCTIONS                             
-- =======================================================================
-- RST
-- .. function:: smooth_move(x, y[, plr = wl.game.Player(1), T = 1000])
--
--    Make a nice moving transition in a given time to the Field(x,y). 
--
--    :arg x, y: Field coordinates to center the view on
--    :type x, y: :class:`integer`
--    :arg plr: The player to move the view for
--    :type plr: :class:`wl.game.Player`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: :const:`nil`
function smooth_move(x, y, g_plr, g_T)
   run(_fm_move, x, y, g_plr, g_T)
end

