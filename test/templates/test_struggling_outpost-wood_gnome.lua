include "test/scripting/check_game_end.lua"

map = game.map

winner = 4
p = game.players[winner]

run(function()
  sleep(1000)
  sf = p:get_buildings("empire_headquarters")[1].fields[1]

  -- These will be 6 fields for 6 foresters we place
  local ff = {}
  for d = 1,6 do
    ff[d] = sf
  end

  -- Move the forester positions 6 steps, each in a different main map direction
  for s = 1, 6 do
    ff[1] = ff[1].ln
    ff[2] = ff[2].tln
    ff[3] = ff[3].trn
    ff[4] = ff[4].rn
    ff[5] = ff[5].brn
    ff[6] = ff[6].bln
  end

  for j,f in ipairs(ff) do
    print(_("Placing forester for the winner at (%d,%d)"):bformat(f.x, f.y))
    b = p:place_building("empire_foresters_house", f, false, true)
    b:set_workers("empire_forester", 1)
    p:conquer(f, 8)
  end

  -- Time limited win condition, let trees grow until the game ends.
end)

check_win_condition(winner)
