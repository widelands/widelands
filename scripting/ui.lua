use("aux", "coroutine")

-- RST
-- ui.lua
-- ---------------
--
-- This script contains one function to make a nice move transition from
-- the players current viewpoint to another place on the map.
--

-- =======================================================================
--                             PRIVATE FUNCTIONS                            
-- =======================================================================
function _calc_move(start, dest, g_T)
   local T = g_T or 1000

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
-- .. function:: timed_move(pts[, dt = 20])
--
--    Moves the view in the given trajectory sleeping dt in between moves. This
--    is most useful in combination with :func:`scroll_smoothly_to`.
--    scroll_smoothly_to moves to a given location and this function can be
--    used to reverse the movement precisely:
--
--    .. code-block:: lua
--
--       use("aux", "table") -- for reverse() 
--
--       pts = scroll_smoothly_to(wl.map.fields(23, 42))
--       sleep(1000)
--       timed_move(array_reverse(pts)) -- Moves in one second back 
--
--    :arg pts: an array of points, that is tables with ``x`` and ``y`` members
--       that define the viewpoints to set the view to.
--    :type pts: :class:`array`
--    :arg dt: Time in ms to sleep between the move points.
--    :type dt: :class:`integer`
function timed_move(points, gdt)
   local dt = gdt or 20
   local mv = wl.ui.MapView()

   for idx,p in ipairs(points) do
      mv.viewpoint_x = p.x
      mv.viewpoint_y = p.y
      
      sleep(dt)
   end
end


-- RST
-- .. function:: scroll_smoothly_to(f[, T = 1000])
--
--    Make a nice moving transition in a given time to the Field(x,y).
--    The function will return as soon as the transition is completed.
--
--    :arg f: Field to center the view on
--    :type r: :class:`wl.map.Field`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function scroll_smoothly_to(f, g_T)
   local mv = wl.ui.MapView()

   local start = {
      x = wl.ui.MapView().viewpoint_x,
      y = wl.ui.MapView().viewpoint_y
   }
   -- Center this field nicely.
   local dest = {
      x = f.viewpoint_x - mv.width / 2,
      y = f.viewpoint_y - mv.height / 2
   }

   pts = _calc_move(start, dest, g_T)
   
   timed_move(pts)

   return pts
end

