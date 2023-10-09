include "test/scripting/check_game_end.lua"

winner = 1

run(function()
  sleep(10000)
  p = game.players[winner]
  b = p:get_buildings(p.tribe.name .. "_headquarters")
  b[1]:set_wares("gold", 1000)
end)

check_win_condition(winner)
