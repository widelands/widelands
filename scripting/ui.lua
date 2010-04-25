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
-- .. function:: timed_scroll(pts[, dt = 20])
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
--       -- Move there in one second
--       pts = scroll_smoothly_to(wl.map.fields(23, 42))
--       -- Move back in one second
--       timed_scroll(array_reverse(pts))
--
--    :arg pts: an array of points, that is tables with ``x`` and ``y`` members
--       that define the viewpoints to set the view to.
--    :type pts: :class:`array`
--    :arg dt: Time in ms to sleep between the move points.
--    :type dt: :class:`integer`
function timed_scroll(points, gdt)
   local dt = gdt or 20
   local mv = wl.ui.MapView()

   for idx,p in ipairs(points) do
      mv.viewpoint_x = p.x
      mv.viewpoint_y = p.y
      
      sleep(dt)
   end
end


-- RST
-- .. function:: scroll_smoothly_to_pos(x, y[, T = 1000])
--
--    Make a nice moving transition in a given time to the viewpoint x, y.
--    The function will return as soon as the transition is completed.
-- 
--    :arg x: x position to center the view on
--    :type x: :class:`integer`
--    :arg y: y position to center the view on
--    :type y: :class:`integer`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function scroll_smoothly_to_pos(x, y, g_T)
   local start = {
      x = wl.ui.MapView().viewpoint_x,
      y = wl.ui.MapView().viewpoint_y
   }
   local dest = { x = x, y = y }

   pts = _calc_move(start, dest, g_T)
   
   timed_scroll(pts)

   return pts
end

-- RST
-- .. function:: scroll_smoothly_to(f[, T = 1000])
--
--    Make a nice moving transition in a given time to center the Field(x,y).
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
   return scroll_smoothly_to_pos(
      f.viewpoint_x - mv.width / 2, f.viewpoint_y - mv.height / 2, g_T
   )
end



-- RST
-- .. function:: timed_mouse(pts[, dt = 20])
--
--    Mouses the cursor in the given trajectory sleeping dt in between moves.
--
--    :arg pts: an array of points, that is tables with ``x`` and ``y`` members
--       that define the intermediate points the mouse should pass through.
--    :type pts: :class:`array`
--    :arg dt: Time in ms to sleep between the move points.
--    :type dt: :class:`integer`
function timed_mouse(points, gdt)
   local dt = gdt or 20
   local mv = wl.ui.MapView()

   for idx,p in ipairs(points) do
      mv.mouse_position_x = p.x
      mv.mouse_position_y = p.y
      
      sleep(dt)
   end
end

-- RST
-- .. function:: mouse_smoothly_to_pos(x, y[, T = 1000])
--
--    Make a nice moving transition for the mouse to the given pixels relative
--    to the top left corner of the screen. 
--
--    :arg x: x position to center the view on
--    :type x: :class:`integer`
--    :arg y: y position to center the view on
--    :type y: :class:`integer`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function mouse_smoothly_to_pos(x, y, g_T)
   local start = {
      x = wl.ui.MapView().mouse_position_x,
      y = wl.ui.MapView().mouse_position_y
   }
   local dest = { x = x, y = y }

   pts = _calc_move(start, dest, g_T)
   
   timed_mouse(pts)

   return pts
end

-- RST
-- .. function:: scroll_smoothly_to(f[, T = 1000])
--
--    Move the mouse on the given field. Make sure that the field is inside
--    the current view area.
--
--    :arg f: Field to mouse to
--    :type r: :class:`wl.map.Field`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function mouse_smoothly_to(f, g_T)
   local mv = wl.ui.MapView()
   return mouse_smoothly_to_pos(
      f.viewpoint_x - mv.viewpoint_x, f.viewpoint_y - mv.viewpoint_y, g_T
   )
end



