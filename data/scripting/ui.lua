include "scripting/coroutine.lua"

-- RST
-- ui.lua
-- ---------------
--
-- This script contains UI related functions like for moving the mouse or the
-- view or clicking on fields and UI elements.
--

-- Wait until the mapview is done animating.
function _await_animation()
   local mv = wl.ui.MapView()
   while mv.is_animating do
      sleep(41)
   end
end

-- NOCOM(#sirver): remove smoothly in here.
-- NOCOM(#sirver): replace everything in this file through engine functions.
-- NOCOM(#sirver): fix documentation
-- RST
-- .. function:: scroll_smoothly_to_view(view)
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
-- NOCOM(#sirver): fix documentation
-- NOCOM(#sirver): rename to scroll_to_viewpoint
function scroll_smoothly_to_view(view)
   _await_animation()
   wl.ui.MapView().view = view;
   _await_animation()
end

-- RST
-- .. function:: scroll_smoothly_to(f)
--
--    Make a nice moving transition to the center of the Field(x,y).
--    The function will return as soon as the transition is completed.
--
--    :arg f: Field to center the view on
--    :type r: :class:`wl.map.Field`
--
--    :returns: the prior view of MapView.
function scroll_smoothly_to(f)
   _await_animation()
   local mv = wl.ui.MapView()
   local view = mv.view;
   mv:center_on(f)
   _await_animation()
   return view
end

-- RST
-- .. function:: mouse_to_pixel(x, y[, T = 1000])
--
--    Make a nice moving transition for the mouse to the given pixels relative
--    to the top left corner of the screen.
--
--    :arg x: x position to move the mouse to
--    :type x: :class:`integer`
--    :arg y: y position to move the mouse to
--    :type y: :class:`integer`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: an :class:`array` with the intermediate points that were targeted
-- NOCOM(#sirver): remove g_T
function mouse_to_pixel(x, y, g_T)
   _await_animation()
   wl.ui.MapView():mouse_to_pixel(x, y)
   _await_animation()
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
-- NOCOM(#sirver): this is the only use of Field.viewpoint_{x,y}
-- NOCOM(#sirver): remove timing?
-- NOCOM(#sirver): all functions in here should _await_animation at the beginning too.
function mouse_smoothly_to(f, g_T)
   _await_animation()
   local mv = wl.ui.MapView()
   if not mv:is_visible(f) then
      scroll_smoothly_to(f)
      return mouse_smoothly_to(f, g_T)
   end

   -- NOCOM(#sirver): rename center_on to center_on_field? find some symmetry in here.
   mv:mouse_to_field(f);
   _await_animation()
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
   _await_animation()
   local x, y = wl.ui.MapView():get_descendant_position(panel)
   return mouse_to_pixel(
      x + panel.width / 2,
      y + panel.height / 2,
      g_T
   )
end

-- RST
-- .. function:: click_building(p, building_name[, T = 1000])
--
--    Click on the first building of the given name for the given player.
--
--    :arg p: Player to search building for.
--    :type p: :class:`wl.game.Player`
--    :arg building_name: Building name to look for.
--    :type building_name: :class:`string`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
--
--    :returns: :const:`true` if a building was clicked
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
--    Goes through all open windows and searches for a button of the given name
--    and, if found, clicks it.
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
--    Closes all currently open windows.
--
--    :returns: :const:`nil`
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
--    :returns: :const:`nil`
--
function wait_for_roadbuilding()
   _await_animation()
   while (wl.ui.MapView().is_building_road) do sleep(2000) end
end


-- RST
-- .. function:: wait_for_roadbuilding_and_scroll(f)
--
--    Sleeps while player is in roadbuilding mode, then calls
--    scroll_smoothly_to(f).
--
--    :returns: an :class:`array` with the intermediate points that
--       were targeted
function wait_for_roadbuilding_and_scroll(f)
   _await_animation()
   wait_for_roadbuilding()
   return scroll_smoothly_to(f)
end
