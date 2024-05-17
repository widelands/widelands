include "test/scripting/lunit.lua"
include "test/scripting/check_game_end.lua"

map = game.map
local atl = game.players[2]

-- Map specific values
local port_x = 9
local port_y = 141
local portspace = map:get_field(port_x, port_y)

-- Main script

run(function()
  sleep(2000)
  print("Place a port for the winner to conquer some land.")
  local port = atl:place_building("atlanteans_port", portspace, false, true)
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
  port:set_wares{ spidercloth = 2, iron = 4, planks = 4 }
  port:set_workers("atlanteans_shipwright", 1)
  local ships = atl:get_ships()
  assert_true(#ships > 3, "## Too few ships for New World starting condition ##")

  for i,ship in ipairs(ships) do
    ship:cancel_expedition()
  end

  port:start_refit_to_warship()

  -- 4 minutes to allow the ship to arrive, we will add the refitting time later
  timeout = game.time + 4 * 60 * 1000

  local has_warship = false
  local refitting_ship = nil
  local name = "a ship"
  while not has_warship and game.time < timeout do
    print(string.bformat("Waiting for %s to refit... time left: %d",
                         name, (timeout - game.time) / 1000))
    sleep(5000)
    if refitting_ship == nil then
      for i,ship in ipairs(ships) do
        if ship.type == "warship" then
          has_warship = true
        else
          if ship.pending_refit == "warship" then
            refitting_ship = ship
            name = ship.shipname
            timeout = game.time + 6 * 60 * 1000
            print(string.bformat("Started refitting ship %s", name))
          end
        end
      end
    else
      if refitting_ship.type == "warship" then
        has_warship = true
      end
    end
  end

  assert_true(has_warship, "## Refitting timed out ##")
  print("Warship is ready")
  -- end of refit test

  -- Time limited win condition, let's just wait.
end)

check_win_condition(2)
