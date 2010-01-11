require 'lunit'

lunit.setprivfenv()
lunit.import "assertions"

coords_creation = lunit.TestCase("Coords creation")
function coords_creation:test_create()
   c = wl.map.Coords(25,32)
   assert_equal(c.x, 25)
   assert_equal(c.y, 32)
end
function coords_creation:test_change()
   c = wl.map.Coords(32,25)
   c.x = 10
   c.y = 11
   assert_equal(c.x, 10)
   assert_equal(c.y, 11)
end

-- create_im = lunit.TestCase("Immovable Creation")
-- function create_im:test_create()
--    imm = wl.map.create_immovable("tree1", 25, 14)
--    assert_true(imm) -- Not nil
--    
--    print(imm:serial())
-- 
-- --    assert_equal(imm.pos.x, 25)
-- --    assert_equal(imm.pos.y, 14)
-- end

-- testcase = lunit.TestCase("Immovable Creation")
-- function testcase:setup()
--   self.foobar = "Hello World"
-- end
-- 
-- function testcase:teardown()
--   -- TODO: delete immovable here
-- end
-- 
-- function testcase:test_creation()
--    wl.map.create_immovable("tree1", 25, 14)
--    rv = wl.map.find_immovable(25, 16, 5, "tree")
--    assert_equal(rv[1], 25)
--    assert_equal(rv[1], 14)
-- end


lunit:run()
