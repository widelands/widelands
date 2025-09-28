print("INFO: run editor_init.lua")  -- printed to show that the script runs twice

if editor_test_has_run then
   print("(quit again)")
   wl.ui.MapView():close() -- was done by editor_test_loading.lua, but must be repeated
end
