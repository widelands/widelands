include "test/scripting/check_game_end.lua"

winner = 3

run(function()
  sleep(10000)

  for i,p in ipairs(game.players) do
    sleep(1000)
    if i ~= winner then
      b = p:get_buildings(p.tribe.name .. "_headquarters")
      -- there shouldn't be more than 1
      b[1]:destroy()
    end
  end
end)

check_win_condition(winner)
