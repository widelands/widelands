include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

-- player 3 is on winning team, but is eliminated
eliminated = {3, 2, 1}
winners = {4, 5}

run(function()
  sleep(8000)

  for i,n in ipairs(eliminated) do
    sleep(2000)
    p = game.players[n]
    print("Destroying HQ of " .. p.name)
    b = p:get_buildings(p.tribe.name .. "_headquarters")
    -- there shouldn't be more than 1
    assert_equal(1, #b, "## HQ not found or more than 1 HQs ##")
    b[1]:destroy()
  end
  print("All loser HQs destroyed, win condition should be triggered now.")
end)

check_win_condition(winners, eliminated)
