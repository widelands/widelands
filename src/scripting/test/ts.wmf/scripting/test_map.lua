-- ============================
-- Test functionality in wl.map 
-- ============================

test_map = lunit.TestCase("Map functions test")

function test_map:test_width()
   assert_equal(64, wl.map.get_width())
end
function test_map:test_height()
   assert_equal(80, wl.map.get_height())
end

