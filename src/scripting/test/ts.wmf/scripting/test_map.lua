-- ============================
-- Test functionality in wl.map 
-- ============================

test_map = lunit.TestCase("Map functions test")

function test_map:test_width()
   assert_equal(64, wl.Map().width)
end
function test_map:test_height()
   assert_equal(80, wl.Map().height)
end

