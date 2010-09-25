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
   imm = map:place_immovable("tree1", map:get_field(9, 10))
   imm2 = map:place_immovable("tree2", map:get_field(10, 10))
   assert_table(imm)
   assert_table(imm2)
   assert_true(imm.serial > 0)
   assert_true(imm2.serial > 0)
   assert_true(imm2.serial > imm.serial)

   imm:remove()
   imm2:remove()
end
function immovable_creation_tests:test_create_tribe_immovables()
   imm = map:place_immovable("field2", map:get_field(10,10), "barbarians")
   imm:remove()
end
function immovable_creation_tests:test_create_world_immovables()
   imm = map:place_immovable("tree1", map:get_field(10,10), "world")
   imm:remove()
end
function immovable_creation_tests:test_create_world_immovables1()
   imm = map:place_immovable("tree1", map:get_field(10,10), nil)
   imm:remove()
end
function immovable_creation_tests:test_create_tribe_immovables_ill_tribe()
   assert_error("Illegal tribe!", function()
      imm = map:place_immovable("field2", map:get_field(10,10), "blablub")
   end)
end
function immovable_creation_tests:test_create_tribe_immovables_ill_immovable()
   assert_error("Illegal tribe!", function()
      imm = map:place_immovable("b", map:get_field(10,10), "barbarians")
   end)
end
function immovable_creation_tests:test_create_wrong_usage()
   assert_error("Needs table, not integer", function()
      map:place_immovable("tree1", 9, 10) end)
end
function immovable_creation_tests:test_create_wrong_usage()
   assert_error("Needs table, not integer", function()
      map:place_immovable("tree1", 9, 10) end)
end

-- ===================
-- Simple usage tests
-- ===================
immovable_tests = lunit.TestCase("Immovable usage")
function immovable_tests:setup()
   self.i = map:place_immovable("tree1", map:get_field(9, 10))
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
   assert_equal("tree1", f.immovable.name)
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
   self.small = map:place_immovable("tree1", map:get_field(18, 10))
   self.medium = player1:place_building(
      "burners_house", map:get_field(10,10), false, true
   )
   self.big = map:place_immovable("stones4", map:get_field(20, 10))
   self.big_building = player1:place_building(
      "fortress", map:get_field(15,11), false, true
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
   assert_equal("none", self.none.size)
end
function immovable_property_tests:test_size_small()
   assert_equal("small", self.small.size)
end
function immovable_property_tests:test_size_medium()
   assert_equal("medium", self.medium.size)
end
function immovable_property_tests:test_size_big()
   assert_equal("big", self.big.size)
end
function immovable_property_tests:test_size_fortress()
   assert_equal("big", self.big_building.size)
end
function immovable_property_tests:test_name_pebble()
   assert_equal("pebble1", self.none.name)
end
function immovable_property_tests:test_name_tree()
   assert_equal("tree1", self.small.name)
end
function immovable_property_tests:test_name_charcoal_burner()
   assert_equal("burners_house", self.medium.name)
end
function immovable_property_tests:test_name_stone()
   assert_equal("stones4", self.big.name)
end
function immovable_property_tests:test_name_fortress()
   assert_equal("fortress", self.big_building.name)
end

function immovable_property_tests:test_type_pebble()
   assert_equal("immovable", self.none.type)
end
function immovable_property_tests:test_type_tree()
   assert_equal("immovable", self.small.type)
end
function immovable_property_tests:test_type_charcoal_burner()
   assert_equal("productionsite", self.medium.type)
   assert_equal("burners_house", self.medium.name)
end
function immovable_property_tests:test_type_stone()
   assert_equal("immovable", self.big.type)
end
function immovable_property_tests:test_type_fortress()
   assert_equal("militarysite", self.big_building.type)
   assert_equal("fortress", self.big_building.name)
end

function immovable_property_tests:test_fields_pebble()
   assert_equal(1, #self.none.fields)
   assert_equal(map:get_field(19,10), self.none.fields[1])
end
function immovable_property_tests:test_fields_tree()
   assert_equal(1, #self.small.fields)
   assert_equal(map:get_field(18,10), self.small.fields[1])
end
function immovable_property_tests:test_fields_charcoal_burner()
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

