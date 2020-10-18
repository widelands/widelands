-- TODO(GunChleoc): Document more fully

-- Close the newest story message box window if there is one
function close_story_messagebox()
   if (wl.ui.MapView().windows.story_message_box ~= nil) then
      wl.ui.MapView().windows.story_message_box:close()
   end
end

-- Waits for a field action tab to be there
function wait_for_field_action_tab(name)
   local mapview = wl.ui.MapView()
   while not mapview.windows.field_action or not mapview.windows.field_action.tabs[name] do
      sleep(100)
   end
end
