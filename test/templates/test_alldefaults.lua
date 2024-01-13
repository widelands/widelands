include "scripting/coroutine.lua"

run(function()
  sleep(1000)
  wl.Game().desired_speed = 5000
  sleep(10000)
  print("# All Tests passed.")
  wl.ui.MapView():close()
end)
