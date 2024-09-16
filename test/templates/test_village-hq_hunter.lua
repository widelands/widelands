include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

-- Slots 3 to 5 are closed, players[3] is slot 6
winner = 3
eliminated = {1, 2, 4, 5}

run(function()
  sleep(8000)

  for i,p in ipairs(game.players) do
    sleep(2000)
    if i ~= winner then
      print("Destroying HQ of " .. p.name)
      b = p:get_buildings(p.tribe.name .. "_headquarters")
      -- there shouldn't be more than 1
      assert_equal(1, #b, "## HQ not found or more than 1 HQs ##")
      b[1]:destroy()
    end
  end
  print("All loser HQs destroyed, win condition should be triggered now.")
end)

check_win_condition(winner, eliminated)
