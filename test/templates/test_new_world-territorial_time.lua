include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

map = game.map
atl = game.players[2]

function f(x, y)
  r = map:get_field(x, y)
  return r
end

run(function()
  sleep(2000)
  print("Place a port for the winner to conquer some land.")
  atl:place_building("atlanteans_port", f(9, 141), false, true)

  -- Test naval warfare
  sleep(1000)
  assert_true(#atl:get_ships() > 3, "## Too few ships for New World starting condition ##")
  local ship = atl:get_ships()[1]
  assert_equal(ship.type, "transport", "## Ship 1 is not a transport ship ##")

  print("Refitting a ship to warship")
  ship:refit("warship")

  local refit_timeout = game.time + 5 * 60 * 1000
  while ship.type == "transport" and game.time < refit_timeout do
    print("Waiting for ship to refit...")
    sleep(5000)
  end

  assert_equal(ship.type, "warship", "## Refitting timed out ##")

  -- Time limited win condition, let's just wait.
end)

check_win_condition(2)
