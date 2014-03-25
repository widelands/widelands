include "scripting/coroutine.lua"

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

   local function s(t)
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
      local cpos = s(t)
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
include "scripting/table.lua" -- for reverse()
--
--       -- Move there in one second
--       pts = scroll_smoothly_to(wl.Game().map:get_field(23, 42))
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

   local pts = _calc_move(start, dest, g_T)

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
   local x, y
   local map = wl.Game().map
   if math.abs(f.viewpoint_x - mv.viewpoint_x) <
      math.abs(f.viewpoint_x + 64 * map.width - mv.viewpoint_x)
   then
      x = f.viewpoint_x
   else
      x = f.viewpoint_x + map.width * 64
   end


   if math.abs(f.viewpoint_y - mv.viewpoint_y) <
      math.abs(f.viewpoint_y + 32 * map.height - mv.viewpoint_y)
   then
      y = f.viewpoint_y
   else
      y = f.viewpoint_y + map.height * 32
   end

   return scroll_smoothly_to_pos(x - mv.width / 2, y - mv.height / 2, g_T)
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
-- .. function:: mouse_smoothly_to(f[, T = 1000])
--
--    Move the mouse on the given field. Make sure that the field is inside
--    the current view area.
--
--    :arg f: Field to mouse to
--    :type f: :class:`wl.map.Field`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function mouse_smoothly_to(f, g_T)
   local mv = wl.ui.MapView()
   local dx, dy = f.viewpoint_x - mv.viewpoint_x,
      f.viewpoint_y - mv.viewpoint_y

   -- FixMe: we need the width and height of triangles here to fix
   -- for situations where we are close to the borders of the map. Because
   -- these functions are not used very often, I decided to enter them here
   -- directly instead of wrapping this up properly. I hope this will not
   -- lead to problems in the future.
   local map = wl.Game().map
   if dx < 0 then dx = dx + map.width * 64 end
   if dy < 0 then dy = dy + map.height * 32 end

   if dx > mv.width or dy > mv.height then
      scroll_smoothly_to(f, g_T)
      return mouse_smoothly_to(f, g_T)
   else
      return mouse_smoothly_to_pos(dx, dy, g_T)
   end
end

-- RST
-- .. function:: mouse_smoothly_to_panel(panel[, T = 1000])
--
--    Move the mouse to the center of the given ui element.
--
--    :arg panel: Panel to mouse to
--    :type panel: :class:`wl.ui.Panel`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were
--       targeted
function mouse_smoothly_to_panel(panel, g_T)
   local x, y = wl.ui.MapView():get_descendant_position(panel)

   return mouse_smoothly_to_pos(
      x + panel.width / 2,
      y + panel.height / 2,
      g_T
   )
end

-- RST
-- .. function:: click_building(p, building_name[, T = 1000])
--
-- 	Click on the first building of the given name for the given player.
--
--    :arg p: Player to search building for.
--    :type p: :class:`wl.game.Player`
--    :arg building_name: Building name to look for.
--    :type building_name: :class:`string`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
-- 	:returns: :const:`true` if a building was clicked
--
function click_building(p, building_name, g_T)
   local building = p:get_buildings(building_name)[1]
   mouse_smoothly_to(building.fields[1], g_T)
   wl.ui.MapView():click(building.fields[1])
   return true
end

-- RST
-- .. function:: click_button(name)
--
-- 	Goes through all open windows and searches for a button of the given name
-- 	and, if found, clicks it.
--
-- 	:arg name: Name of the button to click.
-- 	:type name: :class:`string`.
--
-- 	:returns: :const:`true` if a button was clicked
--
function click_button(name)
   for button_name, button in pairs(wl.ui.MapView().buttons) do
      if button_name == name then
         button:click()
         return true
      end
   end

   for window_name, window in pairs(wl.ui.MapView().windows) do
      for button_name, button in pairs(window.buttons) do
         if button_name == name then
            print(window_name, button_name)
            button:click()
            return true
         end
      end
   end
   return false
end

-- RST
-- .. function:: close_windows()
--
-- 	Closes all currently open windows.
--
-- 	:returns: :const:`nil`
--
function close_windows()
   for k,v in pairs(wl.ui.MapView().windows) do
      v:close()
   end
end
