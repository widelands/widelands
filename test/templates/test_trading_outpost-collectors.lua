include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

winner = 1

run(function()
  sleep(10000)
  p = game.players[winner]
  print("Adding some gold to the winner's HQ")
  b = p:get_buildings(p.tribe.name .. "_headquarters")
  assert_not_equal(0, #b, "## No HQ found ##")
  b[1]:set_wares("gold", 1000)
  -- Time limited win condition, let's just wait.
end)

check_win_condition(winner)
