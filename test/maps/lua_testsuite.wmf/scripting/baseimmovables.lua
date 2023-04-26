-- =======================================================================
--                         BaseImmovable Usage Tests
-- =======================================================================

-- ====================
-- Creation & Deletion
-- ====================
-- Note: these next functions implicitly also check that inheritance
-- works because serial is a property of a MapObject.
immovable_creation_tests = lunit.TestCase("Immovable Creation")
function immovable_creation_tests:test_create()
   imm = map:place_immovable("aspen_summer_old", map:get_field(9, 10))
   imm2 = map:place_immovable("oak_summer_old", map:get_field(10, 10))
   assert_table(imm)
   assert_table(imm2)
   assert_true(imm.serial > 0)
   assert_true(imm2.serial > 0)
   assert_true(imm2.serial > imm.serial)

   imm:remove()
   imm2:remove()
end
function immovable_creation_tests:test_create_tribe_immovables()
   imm = map:place_immovable("wheatfield_harvested", map:get_field(10,10))
   imm:remove()
end
function immovable_creation_tests:test_create_foreign_tribe_immovables()
   imm = map:place_immovable("pond_mature", map:get_field(10,10))
   imm:remove()
end
function immovable_creation_tests:test_create_world_immovables()
   imm = map:place_immovable("aspen_summer_old", map:get_field(10,10))
   imm:remove()
end
function immovable_creation_tests:test_create_tribe_immovables_ill_immovable()
   assert_error("Tribes immovable does not exist!", function()
      imm = map:place_immovable("b", map:get_field(10,10))
   end)
end
function immovable_creation_tests:test_create_wrong_usage()
   assert_error("Needs table, not integer", function()
      map:place_immovable("aspen_summer_old", 9, 10) end)
end
function immovable_creation_tests:test_create_wrong_usage()
   assert_error("Needs table, not integer", function()
      map:place_immovable("aspen_summer_old", 9, 10) end)
end

-- ===================
-- Simple usage tests
-- ===================
immovable_tests = lunit.TestCase("Immovable usage")
function immovable_tests:setup()
   self.i = map:place_immovable("aspen_summer_old", map:get_field(9, 10))
end
function immovable_tests:teardown()
   pcall(self.i.remove, self.i)
end

function immovable_tests:test_wrongusage()
   self.i:remove()
   assert_error("Should not be able to remove an object twice!",
      function() self.i:remove() end
   )
end
function immovable_tests:test_serial_is_readonly()
   assert_error("Serial should be read only", function() self.i.serial = 12 end)
end
function immovable_tests:test_field_access()
   f = map:get_field(9,10)
   assert_equal("aspen_summer_old", f.immovable.descr.name)
end
function immovable_tests:test_map_object_equality()
   f = map:get_field(9,10)
   assert_equal(self.i, f.immovable)
end
function immovable_tests:test_field_immovable_nil_when_not_set()
   f = map:get_field(10,10)
   assert_equal(nil, f.immovable)
end
function immovable_tests:test_field_immovable_is_read_only()
   f = map:get_field(10,10)
   assert_error("f.immovable should be read only!", function()
      f.immovable = self.i
   end)
end

-- ==============
-- Property tests
-- ==============
immovable_property_tests = lunit.TestCase("Immovable sizes")
function immovable_property_tests:setup()
   self.none = map:place_immovable("pebble1", map:get_field(19, 10))
   self.small = map:place_immovable("aspen_summer_old", map:get_field(18, 10))
   self.medium = player1:place_building(
      "barbarians_charcoal_kiln", map:get_field(10,10), false, true
   )
   self.big = map:place_immovable("greenland_rocks4", map:get_field(20, 10))
   self.big_building = player1:place_building(
      "barbarians_fortress", map:get_field(15,11), false, true
   )
end
function immovable_property_tests:teardown()
   pcall(function () self.none:remove() end)
   pcall(function () self.small:remove() end)
   -- remove flag
   pcall(function () self.medium.fields[1].brn.immovable:remove() end)
   pcall(function() self.big:remove() end)
   pcall(function () self.big_building.fields[1].brn.immovable:remove() end)
end

function immovable_property_tests:test_size_none()
   assert_equal("none", self.none.descr.size)
end
function immovable_property_tests:test_size_small()
   assert_equal("small", self.small.descr.size)
end
function immovable_property_tests:test_size_medium()
   assert_equal("medium", self.medium.descr.size)
end
function immovable_property_tests:test_size_big()
   assert_equal("big", self.big.descr.size)
end
function immovable_property_tests:test_size_fortress()
   assert_equal("big", self.big_building.descr.size)
end
function immovable_property_tests:test_name_pebble()
   assert_equal("pebble1", self.none.descr.name)
end
function immovable_property_tests:test_name_tree()
   assert_equal("aspen_summer_old", self.small.descr.name)
end
function immovable_property_tests:test_name_charcoal_kiln()
   assert_equal("barbarians_charcoal_kiln", self.medium.descr.name)
end
function immovable_property_tests:test_name_rock()
   assert_equal("greenland_rocks4", self.big.descr.name)
end
function immovable_property_tests:test_name_fortress()
   assert_equal("barbarians_fortress", self.big_building.descr.name)
end

function immovable_property_tests:test_type_pebble()
   assert_equal("immovable", self.none.descr.type_name)
end
function immovable_property_tests:test_type_tree()
   assert_equal("immovable", self.small.descr.type_name)
end
function immovable_property_tests:test_type_charcoal_kiln()
   assert_equal("productionsite", self.medium.descr.type_name)
   assert_equal("barbarians_charcoal_kiln", self.medium.descr.name)
end
function immovable_property_tests:test_type_stone()
   assert_equal("immovable", self.big.descr.type_name)
end
function immovable_property_tests:test_type_fortress()
   assert_equal("militarysite", self.big_building.descr.type_name)
   assert_equal("barbarians_fortress", self.big_building.descr.name)
end

function immovable_property_tests:test_fields_pebble()
   assert_equal(1, #self.none.fields)
   assert_equal(map:get_field(19,10), self.none.fields[1])
end
function immovable_property_tests:test_fields_tree()
   assert_equal(1, #self.small.fields)
   assert_equal(map:get_field(18,10), self.small.fields[1])
end
function immovable_property_tests:test_fields_charcoal_kiln()
   assert_equal(1, #self.medium.fields)
   assert_equal(map:get_field(10,10), self.medium.fields[1])
end
function immovable_property_tests:test_fields_stone()
   assert_equal(1, #self.big.fields)
   assert_equal(map:get_field(20,10), self.big.fields[1])
end
function immovable_property_tests:test_field_fortress()
   assert_equal(4, #self.big_building.fields)
   assert_equal(map:get_field(15,11), self.big_building.fields[1])
   assert_equal(map:get_field(15,11).ln, self.big_building.fields[2])
   assert_equal(map:get_field(15,11).tln, self.big_building.fields[3])
   assert_equal(map:get_field(15,11).trn, self.big_building.fields[4])
end

-- ================
-- PlayerImmovables
-- ================
plr_immovables_test = lunit.TestCase("Player Immovables")
function plr_immovables_test:setup()
   self.f = player1:place_flag(map:get_field(13,10), 1)
end
function plr_immovables_test:teardown()
   pcall(self.f.remove, self.f)
end

function plr_immovables_test:test_owner()
   assert_equal(player1.number, self.f.owner.number)
   assert_equal(player1, self.f.owner)
end
