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
    local check_interval = 2000
    local pause_timeout = 2 * 60  -- 2 minutes

    local game_ended = false
    local last_gametime = 0
    local pause_counter = 0

    sleep(1000)
    game.desired_speed = 100000

    repeat
      realtime_sleep(check_interval)
      if (game.time > last_gametime) then
        last_gametime = game.time
        pause_counter = 0
      else
        pause_counter = pause_counter + check_interval / 1000
        print(string.bformat("No progress since last check. (%d seconds)", pause_counter))
      end
      game_ended = mapview.windows.game_summary ~= nil
      print("Checking end of game at " .. formatted_time())
    until game_ended or game.time > timeout or pause_counter >= pause_timeout

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
