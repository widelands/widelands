-- ===========
-- Player test 
-- ===========
player_tests = lunit.TestCase("Player tests sizes")
function player_tests:test_number_property()
   assert_equal(1, wl.game.Player(1).number)
end
function player_tests:test_number_property2()
   assert_equal(2, wl.game.Player(2).number)
end
-- TODO: test accessing illegal values for Player

function player_tests:test_create_flag()
   f = wl.map.Field(10,10)
   k = wl.game.Player(1):place_flag(f, true)
   assert_equal(k.player.number, 1)
   k:remove()
   -- TODO: this test doesn't clean up after itself: the player remains owner
   -- TODO: of the field
end
-- This test is currently disabled because of issue #2938438
-- function player_tests:test_create_flag2()
--    f = wl.map.Field(20,10)
--    k = wl.game.Player(2):place_flag(f, true)
--    assert_equal(k.player.number, 2)
--    k:remove()
--    -- TODO: this test doesn't clean up after itself: the player remains owner
--    -- TODO: of the field
-- end
-- TODO: set non forcing placement of flags

function player_tests:test_force_building()
   f = wl.map.Field(10,10)
   k = wl.game.Player(1):place_building("headquarters", f)
   assert_equal(1, k.player.number)
   assert_equal("warehouse", k.building_type)
   k:remove()
   -- TODO: this test doesn't clean up after itself: the player remains owner
   -- TODO: of the field
end

player_allow_buildings_tests = lunit.TestCase("PlayerAllowed Buildings")
function player_allow_buildings_tests:setup()
   self.p = wl.game.Player(1)
   self.p:allow_buildings "all"
end
function player_allow_buildings_tests:teardown()
   self.p:allow_buildings "all"
end
function player_allow_buildings_tests:test_property()
   assert_equal(true, self.p.allowed_buildings.lumberjacks_hut)
   assert_equal(true, self.p.allowed_buildings["quarry"])
end
function player_allow_buildings_tests:test_forbid_all_buildings()
   self.p:forbid_buildings("all")
   for b,v in pairs(self.p.allowed_buildings) do
      assert_equal(false, v, b .. " was not forbidden!")
   end
end
function player_allow_buildings_tests:test_forbid_some_buildings()
   self.p:forbid_buildings{"lumberjacks_hut"}
   assert_equal(false, self.p.allowed_buildings.lumberjacks_hut)
   assert_equal(true, self.p.allowed_buildings["quarry"])
   self.p:forbid_buildings{"quarry", "sentry"}
   assert_equal(false, self.p.allowed_buildings["quarry"])
   assert_equal(false, self.p.allowed_buildings["sentry"])
end
function player_allow_buildings_tests:test_allow_some()
   self.p:forbid_buildings("all")
   self.p:allow_buildings{"quarry", "sentry"}
   assert_equal(false, self.p.allowed_buildings.lumberjacks_hut)
   assert_equal(true, self.p.allowed_buildings["quarry"])
   assert_equal(true, self.p.allowed_buildings["sentry"])
end
function player_allow_buildings_tests:test_forbid_illegal_buildings()
   function a() self.p:forbid_buildings{"lumberjacksjkdhfs_hut"} end
   assert_error("Illegal building!", a)
end
function player_allow_buildings_tests:test_forbid_string_not_all()
   function a() self.p:forbid_buildings "notall"  end
   assert_error("String argument must be all", a)
end

use("map", "test_objectives")

