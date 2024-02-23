include "test/scripting/check_game_end.lua"

map = game.map
fri = game.players[5]
emp = game.players[4]

function f(x, y)
  r = map:get_field(x, y)
  return r
end

run(function()
  print("Placing ports")
  emp:place_building("empire_port", f(9, 141), false, true)

  fri:place_building("frisians_port", f(179, 5), false, true)

  assert_false(
    game.allow_naval_warfare,
    "## Naval warfare is allowed when it should be disabled by template ##"
  )
  -- Ship refitting from lua works regardless of this setting, can't test it here

  for y = 20, 120, 50 do
    for x = 50, 150, 100 do
      sleep(3000)
      print(_("Conquer some land for winner at %d,%d"):bformat(x, y))
      fri:conquer(f(x, y), 33)
    end
  end
  print("Most land belongs to the winner, win condition should be triggered.")
end)

check_win_condition(5)
