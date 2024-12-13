include "test/scripting/check_game_end.lua"

local map = wl.Game().map
local winner = 1
local artifacts = {}

-- gather the artifacts from the map
for x = 0, (map.width - 1) do
  for y = 0, (map.height - 1) do
    local f = map:get_field(x,y)
    local imm = f.immovable
    if imm ~= nil and imm:has_attribute("artifact") and f == imm.fields[1] then
      table.insert(artifacts, { x, y })
    end
  end
end

assert_true(#artifacts > 0, "## No artifacts found on map. ##")

run(function()
  sleep(8000)

  -- conquer the artifacts
  for i,coords in pairs(artifacts) do
    sleep(2000)
    print(string.bformat("Conquering artifact at %d,%d for the winner.", coords[1], coords[2]))
    game.players[winner]:conquer(game.map:get_field(coords[1],coords[2]), 3)
  end

  print("All artifacts are conquered for the winner, win condition should be triggered soon.")
end)

check_win_condition(winner)
