include "scripting/coroutine.lua"
include "test/scripting/lunit.lua"

local game = wl.Game()

run(function()
  sleep(1000)
  assert_true(#game.players > 1, "## Less than 2 players are in the game ##")
  game.desired_speed = 5000
  sleep(10000)
  print("# All Tests passed.")
  wl.ui.MapView():close()
end)
