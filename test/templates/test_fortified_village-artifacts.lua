include "test/scripting/check_game_end.lua"

run(function()
  sleep(10000)

  -- conquer the artifact
  game.players[1]:conquer(game.map:get_field(25,12), 3)
end)

check_win_condition(1)
