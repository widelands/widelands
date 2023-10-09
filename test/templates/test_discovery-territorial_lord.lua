include "test/scripting/check_game_end.lua"

map = game.map
fri = game.players[5]
emp = game.players[4]

function f(x, y)
  r = map:get_field(x, y)
  return r
end

run(function()
  emp:place_building("empire_port", f(9, 141), false, true)

  fri:place_building("frisians_port", f(179, 5), false, true)

  for y = 20, 170, 50 do
    for x = 50, 150, 100 do
      sleep(1000)
      fri:conquer(f(x, y), 40)
    end
  end
end)

check_win_condition(5)
