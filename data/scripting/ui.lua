include "scripting/coroutine.lua"

-- RST
-- ui.lua
-- ---------------
--
-- This script contains UI related functions like for moving the mouse or the
-- view or clicking on fields and UI elements.
--

-- Sleep until we are done animating.
function _await_animation()
   local mv = wl.ui.MapView()
   while mv.is_animating do
      sleep(41)
   end
end

-- RST
-- .. function:: scroll_to_map_pixel(map_pixel)
--
--    Make a nice moving transition to center on the 'map_pixel', which is a table
--    that must contain 'x' and 'y' keys. The function will return as soon as
--    the transition is completed.
--
--    :arg map_pixel: pixel to focus on.
--    :type map_pixel: :class:`table`
--
function scroll_to_map_pixel(map_pixel)
   _await_animation()
   wl.ui.MapView():scroll_to_map_pixel(map_pixel.x, map_pixel.y)
   _await_animation()
end

-- RST
-- .. function:: scroll_to_field(field)
--
--    Make a nice moving transition to center the 'field' on screen. The
--    function will return as soon as the transition is completed.
--
--    :arg field: Field to center the view on
--    :type field: :class:`wl.map.Field`
--
--    :returns: the prior center map pixel of the MapView as a table containing
--       'x' and 'y' keys.
--
function scroll_to_field(field)
   _await_animation()
   local mv = wl.ui.MapView()
   local center_map_pixel = mv.center_map_pixel
   mv:scroll_to_field(field)
   _await_animation()
   return center_map_pixel
end

-- RST
-- .. function:: mouse_to_pixel(x, y)
--
--    Make a nice moving transition for the mouse to the given pixels relative
--    to the top left corner of the screen. The function will return as soon as
--    the transition is completed.
--
--    :arg x: x position to move the mouse to
--    :type x: :class:`integer`
--    :arg y: y position to move the mouse to
--    :type y: :class:`integer`
--
function mouse_to_pixel(x, y)
   _await_animation()
   wl.ui.MapView():mouse_to_pixel(x, y)
   _await_animation()
end

-- RST
-- .. function:: mouse_to_field(field)
--
--    Move the mouse on the given field. Makes sure that the field is inside
--    the current view area by scrolling the view if necessary. The function
--    will return as soon as the transition is completed.
--
--    :arg field: Field to mouse to
--    :type field: :class:`wl.map.Field`
--
function mouse_to_field(field)
   _await_animation()
   local mv = wl.ui.MapView()
   if not mv:is_visible(field) then
      scroll_to_field(field)
      mouse_to_field(field)
      return
   end

   mv:mouse_to_field(field)
   _await_animation()
end

-- RST
-- .. function:: mouse_to_panel(panel)
--
--    Move the mouse to the center of the given ui element. The function will
--    return as soon as the transition is completed.
--
--    :arg panel: Panel to mouse to
--    :type panel: :class:`wl.ui.Panel`
--
function mouse_to_panel(panel)
   _await_animation()
   local x, y = wl.ui.MapView():get_descendant_position(panel)
   mouse_to_pixel(x + panel.width / 2, y + panel.height / 2)
end

-- RST
-- .. function:: click_building(player, building_name)
--
--    Click on the first building of type 'building_name' owned by 'player'.
--
--    :arg player: Player to search building for.
--    :type player: :class:`wl.game.Player`
--    :arg building_name: Building name to look for.
--    :type building_name: :class:`string`
--
--    :returns: :const:`true` if a building was clicked
--
function click_building(player, building_name)
   local building = player:get_buildings(building_name)[1]
   mouse_to_field(building.fields[1])
   wl.ui.MapView():click(building.fields[1])
   return true
end

-- RST
-- .. function:: click_button(name)
--
--    Searches through all open windows for a button named 'name' and, if
--    found, clicks it.
--
--    :arg name: Name of the button to click.
--    :type name: :class:`string`.
--
--    :returns: :const:`true` if a button was clicked
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
--    Closes all open windows.
--
function close_windows()
   for k,v in pairs(wl.ui.MapView().windows) do
      v:close()
   end
end


-- RST
-- .. function:: wait_for_roadbuilding()
--
--    Sleeps while player is in roadbuilding mode.
--
function wait_for_roadbuilding()
   _await_animation()
   while (wl.ui.MapView().is_building_road) do sleep(2000) end
end


-- RST
-- .. function:: wait_for_roadbuilding_and_scroll(field)
--
--    Sleeps while player is in roadbuilding mode, then calls
--    scroll_to_field(field).
--
--    :returns: The return value of `scroll_to_field`.
function wait_for_roadbuilding_and_scroll(field)
   _await_animation()
   wait_for_roadbuilding()
   return scroll_to_field(field)
end
