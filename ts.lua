require 'lunit'

lunit.setprivfenv()
lunit.import "assertions"

coords_tests = lunit.TestCase("Coords creation")
function coords_tests:test_create()
   c = wl.map.Coords(25,32)
   assert_equal(c.x, 25)
   assert_equal(c.y, 32)
end
function coords_tests:test_change()
   c = wl.map.Coords(32,25)
   c.x = 10
   c.y = 11
   assert_equal(c.x, 10)
   assert_equal(c.y, 11)
end
-- TODO: c1 == c2 for two coords

-- Note: these next functions implicitly also check that inheritance
-- works because serial is a property of a MapObject.
immovable_creation_tests = lunit.TestCase("Immovable Creation")
function immovable_creation_tests:test_create()
   imm = wl.map.create_immovable("tree1", 9, 10)
   imm2 = wl.map.create_immovable("tree2", 10, 10)
   assert_table(imm) 
   assert_table(imm2)
   assert_true(imm.serial > 0)
   assert_true(imm2.serial > 0)
   assert_true(imm2.serial > imm.serial)

   imm:remove()
   imm2:remove()
end

-- =======================================================================
--                           Immovable Usage Tests                          
-- =======================================================================

-- ===================
-- Simple usage tests 
-- ===================
immovable_tests = lunit.TestCase("Immovable usage")
function immovable_tests:setup()
   self.i = wl.map.create_immovable("tree1", 9, 10)
end
function immovable_tests:teardown()
   pcall(self.i.remove, self.i)
end

function immovable_tests:test_wrongusage()
   self.i:remove()
   assert_error("Should not be able to remove an object twice!",
      self.i.remove, self.i
   )
end
function immovable_tests:test_serial_is_readonly()
   function setserial(i)
      i.serial = 12
   end
   assert_error("Serial should be read only", setserial, self.i)
end


-- ===========
-- Size tests 
-- ===========
immovable_size_tests = lunit.TestCase("Immovable sizes")
function immovable_size_tests:setup()
   self.none = wl.map.create_immovable("pebble1", 9, 10)
   self.small = wl.map.create_immovable("tree1", 10, 10)
   -- No medium bob in world. Need a user immovable here!!! TODO
   -- self.medium = wl.map.create_immovable("tree1", 10, 10)
   self.big = wl.map.create_immovable("stones4", 15, 10)
end
function immovable_size_tests:teardown()
   pcall(self.none.remove, self.none)
   pcall(self.small.remove, self.small)
   -- pcall(self.medium.remove, self.medium)
   pcall(self.big.remove, self.big)
end

function immovable_size_tests:test_none()
   assert_equal("none", self.none.size)
end
function immovable_size_tests:test_small()
   assert_equal("small", self.small.size)
end
function immovable_size_tests:test_big()
   assert_equal("big", self.big.size)
end

-- function immovable_tests:test_position()
--   assert_equal(self.i.pos.x, 9)
--   assert_equal(self.i.pos.y, 10)
--end



-- function testcase:test_creation()
--    wl.map.create_immovable("tree1", 25, 14)
--    rv = wl.map.find_immovable(25, 16, 5, "tree")
--    assert_equal(rv[1], 25)
--    assert_equal(rv[1], 14)
-- end


lunit:run()
