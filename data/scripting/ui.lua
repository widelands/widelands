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

-- NOCOM(#sirver): fix documentation everywhere in this file.
-- RST
-- .. function:: scroll_to_viewpoint(view)
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
function scroll_to_viewpoint(view)
   _await_animation()
   wl.ui.MapView().view = view;
   _await_animation()
end

-- RST
-- .. function:: scroll_to_field(f)
--
--    Make a nice moving transition to the center of the Field(x,y).
--    The function will return as soon as the transition is completed.
--
--    :arg f: Field to center the view on
--    :type r: :class:`wl.map.Field`
--
--    :returns: the prior view of MapView.
function scroll_to_field(f)
   _await_animation()
   local mv = wl.ui.MapView()
   local view = mv.view;
   mv:center_on(f)
   _await_animation()
   return view
end

-- RST
-- .. function:: mouse_to_pixel(x, y)
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
function mouse_to_pixel(x, y)
   _await_animation()
   wl.ui.MapView():mouse_to_pixel(x, y)
   _await_animation()
end

-- RST
-- .. function:: mouse_to_field(f)
--
--    Move the mouse on the given field. Make sure that the field is inside
--    the current view area.
--
--    :arg f: Field to mouse to
--    :type f: :class:`wl.map.Field`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
function mouse_to_field(f)
   _await_animation()
   local mv = wl.ui.MapView()
   if not mv:is_visible(f) then
      scroll_to_field(f)
      mouse_to_field(f)
      return
   end

   -- NOCOM(#sirver): rename center_on to center_on_field? find some symmetry in here.
   mv:mouse_to_field(f);
   _await_animation()
end

-- RST
-- .. function:: mouse_to_panel(panel)
--
--    Move the mouse to the center of the given ui element.
--
--    :arg panel: Panel to mouse to
--    :type panel: :class:`wl.ui.Panel`
--    :arg T: Time in ms to take for the transition.
--    :type T: :class:`integer`
function mouse_to_panel(panel)
   _await_animation()
   local x, y = wl.ui.MapView():get_descendant_position(panel)
   mouse_to_pixel(x + panel.width / 2, y + panel.height / 2)
end

-- RST
-- .. function:: click_building(p, building_name)
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
function click_building(p, building_name)
   local building = p:get_buildings(building_name)[1]
   mouse_to_field(building.fields[1])
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
-- .. function:: wait_for_roadbuilding_and_scroll(f)
--
--    Sleeps while player is in roadbuilding mode, then calls
--    scroll_to_field(f).
--
--    :returns: the prior view of MapView.
function wait_for_roadbuilding_and_scroll(f)
   _await_animation()
   wait_for_roadbuilding()
   return scroll_to_field(f)
end
