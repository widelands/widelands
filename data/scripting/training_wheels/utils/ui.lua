-- NOCOM document
function close_story_messagebox()
   if (wl.ui.MapView().windows.story_message_box ~= nil) then
      --wl.ui.MapView().windows.story_message_box.buttons.ok:click()
      wl.ui.MapView().windows.story_message_box:close()
      --sleep(100)
   end
end
