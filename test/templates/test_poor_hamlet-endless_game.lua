include "scripting/coroutine.lua"

run(function()
  sleep(10000)
  print("# All Tests passed.")
  wl.ui.MapView():close()
end)
