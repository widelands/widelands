print("INFO: running editor_init.lua")  -- printed to show that the script runs twice

if player1 then -- was loaded before
   print("(quit again)")
   wl.ui.MapView():close() -- was done by editor_test_*.lua, but must be repeated
end
