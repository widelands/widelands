include "map:scripting/common_init.lua"

-- ==========
-- Constants
-- ==========
if not wl.editor then
   game = wl.Game()
   -- make sure we do not crash because of saving.
   game.allow_saving = false
   egbase = game
else
   editor = wl.Editor()
   egbase = editor
end
map = egbase.map

player1 = egbase.players[1]
player2 = egbase.players[2]
player3 = egbase.players[3]

-- TODO(GunChleoc): Editor does not conquer properly
player1:place_building("barbarians_headquarters", map.player_slots[1].starting_field, false, true)

-- For immovables_descriptions.lua
local field = map:get_field(63,79)
if (field.immovable == nil) then
   map:place_immovable("bush1", field)
end
field = map:get_field(62,79)
if (field.immovable == nil) then
   map:place_immovable("alder_summer_sapling", field)
end
field = map:get_field(61,79)
if (field.immovable == nil) then
   map:place_immovable("alder_summer_old", field)
end

-- =================================
-- Tests for the core functionality
-- =================================
include "map:scripting/egbase.lua"

include "map:scripting/gettext.lua"
include "map:scripting/math_random.lua"
include "map:scripting/string_bformat.lua"
include "map:scripting/path.lua"
include "map:scripting/map.lua"
include "map:scripting/cplayer.lua"
include "map:scripting/cfield.lua"
include "map:scripting/cplr_access.lua"

include "map:scripting/immovables.lua"
include "map:scripting/immovables_descriptions.lua"
local ret_empty = include "map:scripting/terrains_resources_descriptions.lua"

local function to_test_include_in_fn()
   ret = include "map:scripting/tribes_descriptions.lua"
   return ret
end
local ret_empty_from_fn = to_test_include_in_fn() -- tested directly

if not wl.editor then
   include "map:scripting/game.lua"

   include "map:scripting/geconomy.lua"
   include "map:scripting/gplayer.lua"
   include "map:scripting/gfield.lua"
   include "map:scripting/gplr_access.lua"

   include "map:scripting/objectives.lua"
   include "map:scripting/messages.lua"
else
   include "map:scripting/editor.lua"
   include "map:scripting/eplr_access.lua"

   include "map:scripting/efield.lua"
end

-- ===========================
-- Test for auxiliary scripts
-- ===========================
local ret_values = include "map:scripting/set.lua"
-- "map:scripting/table.lua" is included in function below
local function test_with_coro(test_case, sleep_time)
   if sleep_time > 0 then
      -- only call sleep in game but not in editor (even with 0)
      sleep(sleep_time)
   end
   local ret = include "map:scripting/table.lua"
   function test_case:test_include_in_coro()
      assert_table(ret)
      assert_nil(next(ret), "empty table")
   end
end
-- is called later

-- ===========================
-- Test for return of include
-- ===========================
local test_include = lunit.TestCase("include tests")

function test_include:test_include_return_nothing()
   assert_table(ret_empty)
   assert_nil(next(ret_empty), "empty table")
end

function test_include:test_include_return_nothing_in_func()
   assert_table(ret_empty_from_fn)
   assert_nil(next(ret_empty_from_fn), "empty table")
end

function test_include:test_include_return_values()
   assert_table(ret_values)
   assert_equal('set return text', ret_values.testString)
   assert_equal(7, ret_values.testNo)
end

local tc_at_end = lunit.TestCase("check at end")
function tc_at_end.test_not_missing()
   -- check that important initial things are still here
   local at_startpos = map.player_slots[1].starting_field.immovable
   assert_equal("barbarians_headquarters", at_startpos and at_startpos.descr.name, "on start field")
end

-- ============
-- Test Runner
-- ============

include "scripting/coroutine.lua"
if wl.editor then
   run(test_with_coro, test_include, 0)  -- no sleeping
   lunit:run()
   wl.ui.MapView():close()
else
   run(test_with_coro, test_include, 10)
   run(function()
      sleep(87)
      lunit:run()
      sleep(1000)
      print("(quit)")
      wl.ui.MapView():close()
   end)
end
