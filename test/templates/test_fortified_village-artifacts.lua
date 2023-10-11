include "test/scripting/check_game_end.lua"

winner = 1

run(function()
  sleep(20000)

  -- conquer the artifact
  print("Conquering the artifact for the winner, win condition should be triggered soon.")
  game.players[winner]:conquer(game.map:get_field(25,12), 3)
end)

check_win_condition(winner)
