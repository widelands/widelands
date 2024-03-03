include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

map = game.map
local atl = game.players[2]

-- Map specific values
local port_x = 9
local port_y = 141
local portspace = map:get_field(port_x, port_y)

local map_w = map.width
local map_h = map.height

local function side_canal_penalty(x)
  if x > 15 then
    return 2 * (x - 15)
  end
  if x < 175 then
    return 2 * (175 - x)
  end
  return 0
end

local function distance_to_port(f)
  local dx = (f.x - port_x) % map_w
  if dx > map_w / 2 then
    dx = map_w - dx
  end
  local dy = (f.y - port_y) % map_h
  if dy > map_h / 2 then
    dy = map_h - dy
  end
  return math.max(dx, dy) + side_canal_penalty(f.x)
end

-- Main script

run(function()
  sleep(2000)
  print("Place a port for the winner to conquer some land.")
  atl:place_building("atlanteans_port", portspace, false, true)
  assert_true(#atl:get_buildings("atlanteans_port") >= 1, "## Port placement failed ##")

  -- Test naval warfare

  -- this is the actual test for the setting
  assert_true(
    game.allow_naval_warfare,
    "## Naval warfare is not allowed when it should be enabled by template ##"
  )

  -- but we also do a refit, which would still work from lua if naval warfare were disabled
  -- TODO(tothxa): this can be removed if a proper test case for naval warfare is added to test/maps
  --               win condition duration can also be decreased then
  local ships = atl:get_ships()
  assert_true(#ships > 3, "## Too few ships for New World starting condition ##")

  -- find nearest ship
  local ship = nil
  local min_distance = 10 * (map_w + map_h)
  for i,s in ipairs(ships) do
    if s.type == "transport" and map:sea_route_exists(s.field, portspace) then
      local current_dist = distance_to_port(s.field)
      if current_dist < min_distance then
        ship = s
        min_distance = current_dist
      end
    end
  end

  assert_not_nil(ship, "## No suitable ship found ##")

  print(string.bformat("Refitting ship at %d,%d (distance %d) to warship",
                       ship.field.x, ship.field.y, min_distance))
  ship:refit("warship")

  timeout = game.time + 3 * 60 * 1000
  while ship.type == "transport" and game.time < timeout do
    print(string.bformat("Waiting for ship to refit... time left: %d   distance: %d",
                         (timeout - game.time) / 1000, distance_to_port(ship.field)))
    sleep(5000)
  end

  assert_equal("warship", ship.type, "## Refitting timed out ##")
  print("Warship is ready")
  -- end of refit test

  -- Time limited win condition, let's just wait.
end)

check_win_condition(2)
