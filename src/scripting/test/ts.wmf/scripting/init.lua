require 'lunit'

lunit.setprivfenv()
lunit.import "assertions"

-- =======================================================================
--                                Coords test                               
-- =======================================================================
coords_tests = lunit.TestCase("Coords creation")
function coords_tests:test_create()
   c = wl.map.Coords(25,32)
   assert_equal(c.x, 25)
   assert_equal(c.y, 32)
end
function coords_tests:test_create_xistobig()
   assert_error("x should be too big", function() wl.map.Coords(64, 23) end)
end
function coords_tests:test_create_yistobig()
   assert_error("y should be too big", function() wl.map.Coords(25, 64) end)
end
function coords_tests:test_create_xisnegativ()
   assert_error("x is negativ", function() wl.map.Coords(-12, 23) end)
end
function coords_tests:test_create_yisnegativ()
   assert_error("y is negativ", function() wl.map.Coords(25, -12) end)
end
function coords_tests:test_direct_change_impossible()
   assert_error("c.x should be read only", function() c.x = 12 end) 
   assert_error("c.y should be read only", function() c.y = 12 end) 
end
function coords_tests:test_r_neighbour()
   c = wl.map.Coords(25,40)
   assert_equal(wl.map.Coords(26,40), c.rn)
   c = wl.map.Coords(63,40)
   assert_equal(wl.map.Coords(0,40), c.rn)
end
function coords_tests:test_l_neighbour()
   c = wl.map.Coords(25,40)
   assert_equal(wl.map.Coords(24,40), c.ln)
   c = wl.map.Coords(0,40)
   assert_equal(wl.map.Coords(63,40), c.ln)
end
function coords_tests:test_trn_neighbour()
   c = wl.map.Coords(25,40)
   assert_equal(wl.map.Coords(25,39), c.trn)
   assert_equal(wl.map.Coords(26,38), c.trn.trn)
   assert_equal(wl.map.Coords(26,37), c.trn.trn.trn)
   c = wl.map.Coords(0,0)
   assert_equal(wl.map.Coords(0,63), c.trn)
   assert_equal(wl.map.Coords(1,62), c.trn.trn)
   assert_equal(wl.map.Coords(1,61), c.trn.trn.trn)
end
function coords_tests:test_tln_neighbour()
   c = wl.map.Coords(25,40)
   assert_equal(wl.map.Coords(24,39), c.tln)
   assert_equal(wl.map.Coords(24,38), c.tln.tln)
   assert_equal(wl.map.Coords(23,37), c.tln.tln.tln)
   c = wl.map.Coords(1,1)
   assert_equal(wl.map.Coords(1,0), c.tln)
   assert_equal(wl.map.Coords(0,63), c.tln.tln)
   assert_equal(wl.map.Coords(0,62), c.tln.tln.tln)
end
function coords_tests:test_bln_neighbour()
   c = wl.map.Coords(26,37)
   assert_equal(wl.map.Coords(26,38), c.bln)
   assert_equal(wl.map.Coords(25,39), c.bln.bln)
   assert_equal(wl.map.Coords(25,40), c.bln.bln.bln)

   c = wl.map.Coords(1,61)
   assert_equal(wl.map.Coords(1,62), c.bln)
   assert_equal(wl.map.Coords(0,63), c.bln.bln)
   assert_equal(wl.map.Coords(0,0), c.bln.bln.bln)
end
function coords_tests:test_brn_neighbour()
   c = wl.map.Coords(35,22)
   assert_equal(wl.map.Coords(35,23), c.brn)
   assert_equal(wl.map.Coords(36,24), c.brn.brn)
   assert_equal(wl.map.Coords(36,25), c.brn.brn.brn)

   c = wl.map.Coords(63,63)
   assert_equal(wl.map.Coords(0,0), c.brn)
   assert_equal(wl.map.Coords(0,1), c.brn.brn)
   assert_equal(wl.map.Coords(1,2), c.brn.brn.brn)
end
function coords_tests:test_equality()
   c = wl.map.Coords(32,33)
   c1 = wl.map.Coords(32,33)

   assert_equal(c,c1)
end
function coords_tests:test_inequality()
   c = wl.map.Coords(32,33)
   c1 = wl.map.Coords(33,32)

   assert_not_equal(c,c1)
end

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
   imm = wl.map.create_immovable("tree1", wl.map.Coords(9, 10))
   imm2 = wl.map.create_immovable("tree2", wl.map.Coords(10, 10))
   assert_table(imm) 
   assert_table(imm2)
   assert_true(imm.serial > 0)
   assert_true(imm2.serial > 0)
   assert_true(imm2.serial > imm.serial)

   imm:remove()
   imm2:remove()
end

-- ===================
-- Simple usage tests 
-- ===================
immovable_tests = lunit.TestCase("Immovable usage")
function immovable_tests:setup()
   self.i = wl.map.create_immovable("tree1", wl.map.Coords(9, 10))
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
   function setserial(i)
      i.serial = 12
   end
   assert_error("Serial should be read only", setserial, self.i)
end


-- ==============
-- Property tests 
-- ==============
immovable_property_tests = lunit.TestCase("Immovable sizes")
function immovable_property_tests:setup()
   self.none = wl.map.create_immovable("pebble1", wl.map.Coords(9, 10))
   self.small = wl.map.create_immovable("tree1", wl.map.Coords(10, 10))
   -- No medium bob in world. Need a user immovable here!!! TODO
   -- self.medium = wl.map.create_immovable("tree1", 10, 10)
   self.big = wl.map.create_immovable("stones4", wl.map.Coords(15, 10))
end
function immovable_property_tests:teardown()
   pcall(self.none.remove, self.none)
   pcall(self.small.remove, self.small)
   -- pcall(self.medium.remove, self.medium)
   pcall(self.big.remove, self.big)
end

function immovable_property_tests:test_size_none()
   assert_equal("none", self.none.size)
end
function immovable_property_tests:test_size_small()
   assert_equal("small", self.small.size)
end
function immovable_property_tests:test_size_big()
   assert_equal("big", self.big.size)
end
function immovable_property_tests:test_name_pebble()
   assert_equal("pebble1", self.none.name)
end
function immovable_property_tests:test_nsme_tree()
   assert_equal("tree1", self.small.name)
end
function immovable_property_tests:test_name_stone()
   assert_equal("stones4", self.big.name)
end

-- ====================
-- Find immovables test
-- ====================
-- find_immovables = lunit.TestCase("Find immovables")
-- function immovable_property_tests:teardown()
--    pcall(self.none.remove, self.none)
--    pcall(self.small.remove, self.small)
--    -- pcall(self.medium.remove, self.medium)
--    pcall(self.big.remove, self.big)
-- end

function 
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

rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the game
   wl.debug.exit()
end

