include("scripting/coroutine.lua")

run(function()
  sleep(3000)
  print('# All Tests passed.')
  wl.ui.MapView():close()
end)
