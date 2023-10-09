include "test/scripting/check_game_end.lua"

-- player 3 is on winning team, but is eliminated
losers = {3, 2, 1}
winners = {4, 5}

run(function()
  sleep(10000)

  for i,n in ipairs(losers) do
    sleep(20000)
    p = game.players[n]
    b = p:get_buildings(p.tribe.name .. "_headquarters")
    -- there shouldn't be more than 1
    b[1]:destroy()
  end
end)

check_win_condition(winners)
