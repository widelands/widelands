use("aux", "coroutine")

-- RST
-- smooth_move.lua
-- ---------------
--
-- This script contains one function to make a nice move transition from
-- the players current viewpoint to another place on the map.
--

-- =======================================================================
--                             PRIVATE FUNCTIONS                            
-- =======================================================================
function _calc_move(f, g_plr, g_T)
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
   local rv = {}
   while t < T do
      cpos = s(t)
      rv[#rv+1] = {
         x = cpos.x,
         y = cpos.y,
      }
      t = t + dt
   end

   -- Correct some numerical instabilities
   rv[#rv+1] = {
      x = dest.x,
      y = dest.y
   }

   return rv
end

-- =======================================================================
--                             PUBLIC FUNCTIONS                             
-- =======================================================================

-- RST
-- .. function:: timed_move(pts, [, plr = wl.game.Player(1), dt = 20])
--
--    Moves the view in the given trajectory sleeping dt in between moves. This
--    is most useful in combination with :func:`smooth_move`. smooth_move moves
--    to a given location and this function can be used to reverse the movement
--    precisely:
--
--    .. code-block:: lua
--
--       use("aux", "table") -- for reverse() 
--
--       pts = smooth_move(wl.map.fields(23, 42))
--       sleep(1000)
--       timed_move(reverse(pts)) -- Moves in one second back 
--
--    :arg pts: an array of points, that is tables with ``x`` and ``y`` members
--       that define the viewpoints to set the view to.
--    :type pts: :class:`array`
--    :arg plr: The player to move the view for
--    :type plr: :class:`wl.game.Player`
--    :arg dt: Time in ms to sleep between the move points.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function timed_move(points, gplr, gdt)
   local plr = g_plr or wl.game.Player(1)
   local dt = gdt or 20

   for idx,p in ipairs(points) do
      plr.viewpoint_x = p.x
      plr.viewpoint_y = p.y
      
      sleep(dt)
   end
end


-- RST
-- .. function:: smooth_move(f, [, plr = wl.game.Player(1), T = 1000])
--
--    Make a nice moving transition in a given time to the Field(x,y). 
--
--    :arg f: Field to center the view on
--    :type r: :class:`wl.map.Field`
--    :arg plr: The player to move the view for
--    :type plr: :class:`wl.game.Player`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function smooth_move(f, g_plr, g_T)
   pts = _calc_move(f, g_plr, g_T)
   run(timed_move, pts)
   return pts
end

