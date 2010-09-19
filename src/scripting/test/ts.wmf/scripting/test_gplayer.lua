-- ======================================================
-- Testing Player functionality that is only in the game 
-- ======================================================

function player_tests:test_create_flag()
   local f = map:get_field(10,10)
   local k = player1:place_flag(f, true)
   assert_equal(k.player.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing()
   local f = map:get_field(10,10)
   -- First force the flag, then remove them again
   local k = player1:place_flag(f, true)
   k:remove()
   -- Now, try again, but non forcing
   k = player1:place_flag(f)
   assert_equal(k.player.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing_too_close()
   f = map:get_field(10,10)
   -- First force the flag, then remove them again
   player1:place_flag(f, true):remove()
   player1:place_flag(f.rn, true):remove()

   -- Now, try again, but non forcing
   local k = player1:place_flag(f)
   assert_error("Too close to other!", function() player1:place_flag(f.rn) end)
   k:remove()
end
-- This test is currently disabled because of bug lp:536366
-- function player_tests:test_create_flag2()
--    local f = map:get_field(20,10)
--    local k = wl.Game().players[2]:place_flag(f, true)
--    assert_equal(k.player.number, 2)
--    k:remove()
-- end

function player_tests:test_force_building()
   local f = map:get_field(10,10)
   local k = player1:place_building("headquarters", f)
   assert_equal(1, k.player.number)
   assert_equal("warehouse", k.building_type)
   f.brn.immovable:remove() -- removing map also removes building
end
function player_tests:test_force_building_illegal_name()
   assert_error("Illegal building", function()
      player1:place_building("kjhsfjkh", map:get_field(10,10))
   end)
end


-- -- =======================================================================
-- --                          See Fields/Hide Fields
-- -- =======================================================================
player_vision_tests = lunit.TestCase("Player vision tests")
function player_vision_tests:setup()
   self.f = map:get_field(50, 20)
   player1.see_all = false
end
function player_vision_tests:teardown()
   player1:hide_fields(self.f:region(1))
   player1.see_all = false
end
-- This test must appear as the very first
function player_vision_tests:test_seen_field()
   assert_equal(false, player1:sees_field(self.f))
   assert_equal(false, player1:seen_field(self.f))
   player1:reveal_fields(self.f:region(1))
   player1:hide_fields(self.f:region(1))
   assert_equal(false, player1:sees_field(self.f))
   assert_equal(true, player1:seen_field(self.f))
end

function player_vision_tests:test_sees_field()
   assert_equal(false, player1:sees_field(self.f))
   player1:reveal_fields(self.f:region(1))
   assert_equal(true, player1:sees_field(self.f))
   player1:hide_fields(self.f:region(1))
   assert_equal(false, player1:sees_field(self.f))
end
function player_vision_tests:test_see_all()
   assert_equal(false, player1:sees_field(self.f))
   player1.see_all = true
   assert_equal(true, player1.see_all)
   assert_equal(true, player1:sees_field(self.f))
   player1.see_all = false
   assert_equal(false, player1:sees_field(self.f))
end
function player_vision_tests:test_sees_field_see_all_hide()
   player1.see_all = true
   assert_equal(true, player1:sees_field(self.f))
   player1:hide_fields(self.f:region(1))
   assert_equal(true, player1:sees_field(self.f))
   player1.see_all = false
   assert_equal(false, player1:sees_field(self.f))

   player1:reveal_fields(self.f:region(1))
   assert_equal(true, player1:sees_field(self.f))
   player1.see_all = false
   assert_equal(true, player1:sees_field(self.f))
end


-- =========================
-- Forbid & Allow buildings
-- =========================
player_allow_buildings_tests = lunit.TestCase("PlayerAllowed Buildings")
function player_allow_buildings_tests:setup()
   player1:allow_buildings "all"
end
function player_allow_buildings_tests:teardown()
   player1:allow_buildings "all"
end
function player_allow_buildings_tests:test_property()
   assert_equal(true, player1.allowed_buildings.lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["quarry"])
end
function player_allow_buildings_tests:test_forbid_all_buildings()
   player1:forbid_buildings("all")
   for b,v in pairs(player1.allowed_buildings) do
      assert_equal(false, v, b .. " was not forbidden!")
   end
end
function player_allow_buildings_tests:test_forbid_some_buildings()
   player1:forbid_buildings{"lumberjacks_hut"}
   assert_equal(false, player1.allowed_buildings.lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["quarry"])
   player1:forbid_buildings{"quarry", "sentry"}
   assert_equal(false, player1.allowed_buildings["quarry"])
   assert_equal(false, player1.allowed_buildings["sentry"])
end
function player_allow_buildings_tests:test_allow_some()
   player1:forbid_buildings("all")
   player1:allow_buildings{"quarry", "sentry"}
   assert_equal(false, player1.allowed_buildings.lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["quarry"])
   assert_equal(true, player1.allowed_buildings["sentry"])
end
function player_allow_buildings_tests:test_forbid_illegal_buildings()
   function a() player1:forbid_buildings{"lumberjacksjkdhfs_hut"} end
   assert_error("Illegal building!", a)
end
function player_allow_buildings_tests:test_forbid_string_not_all()
   function a() player1:forbid_buildings "notall"  end
   assert_error("String argument must be all", a)
end

-- ================
-- Access to players buildings
-- ================
player_building_access = lunit.TestCase("Access to Player buildings")
function player_building_access:teardown()
   for temp,b in ipairs(self.bs) do
      pcall(b.remove, b)
   end
end
function player_building_access:test_single()
   self.bs = {
      player1:place_building("lumberjacks_hut", map:get_field(10,10)),
      player1:place_building("lumberjacks_hut", map:get_field(13,10)),
      player1:place_building("quarry", map:get_field(8,10)),
   }
   assert_equal(2, #player1:get_buildings("lumberjacks_hut"))
   assert_equal(1, #player1:get_buildings("quarry"))
end
function player_building_access:test_multi()
   self.bs = {
      player1:place_building("lumberjacks_hut", map:get_field(10,10)),
      player1:place_building("lumberjacks_hut", map:get_field(13,10)),
      player1:place_building("quarry", map:get_field(8,10)),
   }
   rv = player1:get_buildings{"lumberjacks_hut", "quarry"}

   assert_equal(2, #rv.lumberjacks_hut)
   assert_equal(1, #rv.quarry)
end
function player_building_access:test_access()
   local b1 = player1:place_building("lumberjacks_hut", map:get_field(10,10))
   local b2 = player1:place_building("lumberjacks_hut", map:get_field(13,10))
   local b3 = player1:place_building("quarry", map:get_field(8,10))
   self.bs = { b1, b2, b3 }
   rv = player1:get_buildings{"lumberjacks_hut", "quarry"}

   assert_equal(b3, rv.quarry[1])
   b1:remove()
   assert_equal(1, #player1:get_buildings("lumberjacks_hut"))
end

