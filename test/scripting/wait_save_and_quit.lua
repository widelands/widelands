include "scripting/coroutine.lua"
include "test/scripting/stable_save.lua"

local game = wl.Game()

run(function()
  sleep(3000)
  game.desired_speed = 20000
  sleep(120000)
  stable_save(game, "test_save", 1000)

  sleep(3000)
  print('# All Tests passed.')
  wl.ui.MapView():close()
end)
