include "scripting/coroutine.lua"
include "test/scripting/lunit.lua"

game = wl.Game()
mapview = wl.ui.MapView()

local function result_table(winners)
  local r = {}

  for i = 1, #game.players do
    r[i] = 0
  end

  if type(winners) == "number" then
    r[winners] = 1
  else
    for i,w in ipairs(winners) do
      r[w] = 1
    end
  end

  return r
end

local time_fmt = "%02d:%06.3f"
local function formatted_time()
  local t = game.time / 1000
  local m = math.floor(t / 60)
  local s = t % 60
  return time_fmt:bformat(m, s)
end

function check_win_condition(winners)
  run(function()
    -- 2 minutes grace period
    local timeout = (game.win_condition_duration + 2) * 60 * 1000

    local game_ended = false

    sleep(1000)
    game.desired_speed = 100000

    repeat
      sleep(5000)
      game_ended = mapview.windows.game_summary ~= nil
      print("Checking end of game at " .. formatted_time())
    until game_ended or game.time > timeout

    -- Check timeout
    assert_true(game_ended, "## Game did not end in time. ##")

    -- Check that the expected player(s) won
    if (winners == nil) then
      print("### WARNING: check_win_condition():")
      print("###    For better test coverage, please call with list of expected winners ###")
    else
      local expected = result_table(winners)
      for i = 1, #game.players do
        assert_equal(expected[i], game.players[i].end_result,
                     "Wrong result for " .. game.players[i].name)
      end
    end

    print("# All Tests passed.")
    mapview:close()
  end)
end
