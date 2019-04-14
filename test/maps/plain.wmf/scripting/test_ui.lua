local function open_and_close_sound_options(dropdown)
   sleep(100)

   -- Validate selection in dropdown without opening it first
   assert_error("selecting item 0 should have been out of range", function()
      dropdown:select_item(0)
   end)
   assert_error("selecting item 2000 should have been out of range", function()
      dropdown:select_item(2000)
   end)

   dropdown:select_item(1)
   sleep(100)

   local windows = wl.ui.MapView().windows
   window = windows["sound_options_menu"]
   assert_not_nil(window, "Failed to open sound options window")
   sleep(100)
   window:close()

   sleep(100)
   windows = wl.ui.MapView().windows
   assert_nil(windows["sound_options_menu"], "Failed to close sound options window")
end

run(function()
   sleep(100)

   -- Validate listing dropdowns
   local dropdowns = wl.ui.MapView().dropdowns
   for name,dropdown in pairs(dropdowns) do
      assert_equal(name, dropdown.name)
   end

   -- Validate opening and selection
   local dropdown = dropdowns["dropdown_menu_main"]
   assert_not_nil(dropdown, "Failed to find main menu dropdown")
   dropdown:open()
   sleep(100)

   open_and_close_sound_options(dropdown);

   -- Validate selection without opening
   open_and_close_sound_options(dropdown);

   print("# All Tests passed.")

   -- Exit by dropdown
   dropdown:select_item(3)
   sleep(100)

   local message_box = wl.ui.MapView().windows["message_box"]
   assert_not_nil(window, "Failed to exit confirm message box")
   local ok_button = message_box.buttons["ok"]
   assert_not_nil(ok_button, "Exit confirm message box has no 'ok' button")
   ok_button:click()
   sleep(100)

   assert_nil(wl.ui.MapView(), "Exiting by main menu did not close the map view")

   wl.ui.MapView():close()
end)
