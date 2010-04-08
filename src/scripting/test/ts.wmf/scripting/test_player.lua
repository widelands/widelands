-- ===========
-- Player test 
-- ===========

-- NOTE: Most of these tests conquer some area for the player and even though
-- all buildings are destroyed after each test, the area is not unconquered
-- since this functionality is not defined inside the game. This means that the
-- state of these fields bleed into others, therefore for the tree players,
-- there are individual fields which are used only for these tests here, so
-- this bleeding doesn't matter.
player_tests = lunit.TestCase("Player tests")
function player_tests:test_tribe_property()
   assert_equal("barbarians", wl.game.Player(1).tribe)
   assert_equal("empire", wl.game.Player(2).tribe)
   assert_equal("atlanteans", wl.game.Player(3).tribe)
end

function player_tests:test_number_property()
   assert_equal(1, wl.game.Player(1).number)
   assert_equal(2, wl.game.Player(2).number)
   assert_equal(3, wl.game.Player(3).number)
end
function player_tests:test_negativ_val_for_player()
   assert_error("Illegal plr",  function() wl.game.Player(-1) end)
end
function player_tests:test_too_high_val()
   assert_error("Illegal plr",  function() wl.game.Player(10) end)
end
function player_tests:test_ok_val_for_non_existing()
   assert_error("non existing plr",  function() wl.game.Player(5) end)
end

function player_tests:test_create_flag()
   f = wl.map.Field(10,10)
   k = wl.game.Player(1):place_flag(f, true)
   assert_equal(k.player.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing()
   f = wl.map.Field(10,10)
   -- First force the flag, then remove them again
   k = wl.game.Player(1):place_flag(f, true)
   k:remove()
   -- Now, try again, but non forcing
   k = wl.game.Player(1):place_flag(f)
   assert_equal(k.player.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing_too_close()
   f = wl.map.Field(10,10)
   -- First force the flag, then remove them again
   wl.game.Player(1):place_flag(f, true):remove()
   wl.game.Player(1):place_flag(f.rn, true):remove()

   -- Now, try again, but non forcing
   k = wl.game.Player(1):place_flag(f)
   assert_error("Too close to other!", function()
      wl.game.Player(1):place_flag(f.rn)
   end)

   k:remove()
end
-- This test is currently disabled because of issue #2938438
-- function player_tests:test_create_flag2()
--    f = wl.map.Field(20,10)
--    k = wl.game.Player(2):place_flag(f, true)
--    assert_equal(k.player.number, 2)
--    k:remove()
-- end

function player_tests:test_force_building()
   f = wl.map.Field(10,10)
   k = wl.game.Player(1):place_building("headquarters", f)
   assert_equal(1, k.player.number)
   assert_equal("warehouse", k.building_type)
   f.brn.immovable:remove() -- removing map also removes building
end
function player_tests:test_force_building_illegal_name()
   assert_error("Illegal building", function()
      wl.game.Player(1):place_building("kjhsfjkh", wl.map.Field(10,10))
   end)
end


-- -- =======================================================================
-- --                          See Fields/Hide Fields
-- -- =======================================================================
player_vision_tests = lunit.TestCase("Player vision tests")
function player_vision_tests:setup()
   self.f = wl.map.Field(50, 20)
   self.p = wl.game.Player(1)
   self.p.see_all = false
end
function player_vision_tests:teardown()
   self.p:hide_fields(self.f:region(1))
   self.p.see_all = false
end
-- This test must appear as the very first
-- TODO: this test fails in the editor but it should pass. Commented for now
-- function player_vision_tests:test_seen_field()
--    assert_equal(false, self.p:sees_field(self.f))
--    assert_equal(false, self.p:seen_field(self.f))
--    self.p:reveal_fields(self.f:region(1))
--    self.p:hide_fields(self.f:region(1))
--    assert_equal(false, self.p:sees_field(self.f))
--    assert_equal(true, self.p:seen_field(self.f))
-- end

function player_vision_tests:test_sees_field()
   assert_equal(false, self.p:sees_field(self.f))
   self.p:reveal_fields(self.f:region(1))
   assert_equal(true, self.p:sees_field(self.f))
   self.p:hide_fields(self.f:region(1))
   assert_equal(false, self.p:sees_field(self.f))
end
function player_vision_tests:test_see_all()
   assert_equal(false, self.p:sees_field(self.f))
   self.p.see_all = true
   assert_equal(true, self.p.see_all)
   assert_equal(true, self.p:sees_field(self.f))
   self.p.see_all = false
   assert_equal(false, self.p:sees_field(self.f))
end
function player_vision_tests:test_sees_field_see_all_hide()
   self.p.see_all = true
   assert_equal(true, self.p:sees_field(self.f))
   self.p:hide_fields(f:region(1))
   assert_equal(true, self.p:sees_field(self.f))
   self.p.see_all = false
   assert_equal(false, self.p:sees_field(self.f))

   self.p:reveal_fields(self.f:region(1))
   assert_equal(true, self.p:sees_field(self.f))
   self.p.see_all = false
   assert_equal(true, self.p:sees_field(self.f))
end


-- =========================
-- Forbid & Allow buildings
-- =========================
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

-- -- ================
-- -- Access to players buildings
-- -- ================
-- player_building_access = lunit.TestCase("Access to Player buildings")
-- function player_building_access:setup()
--    self.p = wl.game.Player(1)
-- end
-- function player_building_access:teardown()
--    for temp,b in ipairs(self.bs) do
--       pcall(b.remove, b)
--    end
-- end
-- function player_building_access:test_single()
--    self.bs = {
--       self.p:place_building("lumberjacks_hut", wl.map.Field(10,10)),
--       self.p:place_building("lumberjacks_hut", wl.map.Field(13,10)),
--       self.p:place_building("quarry", wl.map.Field(8,10)),
--    }
--    assert_equal(2, #self.p:get_buildings("lumberjacks_hut"))
--    assert_equal(1, #self.p:get_buildings("quarry"))
-- end
-- function player_building_access:test_multi()
--    self.bs = {
--       self.p:place_building("lumberjacks_hut", wl.map.Field(10,10)),
--       self.p:place_building("lumberjacks_hut", wl.map.Field(13,10)),
--       self.p:place_building("quarry", wl.map.Field(8,10)),
--    }
--    rv = self.p:get_buildings{"lumberjacks_hut", "quarry"}
--
--    assert_equal(2, #rv.lumberjacks_hut)
--    assert_equal(1, #rv.quarry)
-- end
-- function player_building_access:test_access()
--    local b1 = self.p:place_building("lumberjacks_hut", wl.map.Field(10,10))
--    local b2 = self.p:place_building("lumberjacks_hut", wl.map.Field(13,10))
--    local b3 = self.p:place_building("quarry", wl.map.Field(8,10))
--    self.bs = { b1, b2, b3 }
--    rv = self.p:get_buildings{"lumberjacks_hut", "quarry"}
--
--    assert_equal(b3, rv.quarry[1])
--    b1:remove()
--    assert_equal(1, #self.p:get_buildings("lumberjacks_hut"))
-- end
--
--
--
--
-- use("map", "test_objectives")
-- use("map", "test_messages")
--
