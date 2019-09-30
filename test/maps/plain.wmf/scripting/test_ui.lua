-- There are more UI test in lua_testsuite.wmf/ui.lua

local function open_and_close_sound_options(dropdown, is_debug_build)
   sleep(100)

   -- Test out-of-range selection in dropdown
   assert_error("Highlighting item 0 should have been out of range", function()
      dropdown:highlight_item(0)
   end)
   assert_error("Highlighting item 2000 should have been out of range", function()
      dropdown:highlight_item(2000)
   end)

   -- Test selecting an item in the dropdown
   if (is_debug_build) then
      dropdown:highlight_item(2)
   else
      dropdown:highlight_item(1)
   end
   assert_nil(wl.ui.MapView().windows.sound_options_menu, "Sound options window should not have been there yet")

   dropdown:select()
   sleep(100)

   window = wl.ui.MapView().windows.sound_options_menu
   assert_not_nil(window, "Failed to open sound options window")
   window:close()

   sleep(100)
   assert_nil(wl.ui.MapView().windows.sound_options_menu, "Failed to close sound options window")
end

run(function()
   sleep(100)

   -- Validate listing dropdowns
   local dropdowns = wl.ui.MapView().dropdowns
   for name,dropdown in pairs(dropdowns) do
      assert_equal(name, dropdown.name)
   end

   -- Validate dropdown functions
   local dropdown = dropdowns["dropdown_menu_main"]
   assert_not_nil(dropdown, "Failed to find main menu dropdown")

   local is_debug_build = dropdown.no_of_items == 4

   -- Selecting from closed dropdown should fail silently
   dropdown:select()

   -- Validate selection without opening
   open_and_close_sound_options(dropdown, is_debug_build);

   -- Validate selection with opening
   dropdown:open()
   sleep(100)
   open_and_close_sound_options(dropdown, is_debug_build);

   -- Exit by dropdown
   local dropdown = dropdowns["dropdown_menu_main"]
      if (is_debug_build) then
      dropdown:highlight_item(4)
   else
      dropdown:highlight_item(3)
   end
   dropdown:select()

   local message_box = wl.ui.MapView().windows["message_box"]
   assert_not_nil(message_box, "Failed to find exit confirm message box")
   local ok_button = message_box.buttons["ok"]
   assert_not_nil(ok_button, "Exit confirm message box has no 'ok' button")

   -- We have to print this before closing the main view, otherwise the test suite
   -- will fail this test, because printing to console won't work.
   print("# All Tests passed.")

   ok_button:click()

   -- Give Widelands some time to close the map view
   local counter = 0
   while (wl.ui.MapView() ~= nil and counter < 50) do
      sleep(1000)
      counter = counter + 1
   end
   assert_nil(wl.ui.MapView(), "Exiting by main menu did not close the map view")

   if (wl.ui.MapView() ~= nil) then
      wl.ui.MapView():close()
   end
end)
