-- No logic required. Just try to load this old map in the Editor.

if editor_init_has_run then -- was loaded before
   -- avoid reporting twice (for cleanness), which happens in editor
   wl.ui.MapView():close() -- must be repeated
   return
end

print("# All Tests passed.")
wl.ui.MapView():close()
editor_init_has_run = 1
