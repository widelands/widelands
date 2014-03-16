-- =============================
-- Test path module
-- =============================
path_test = lunit.TestCase("math.random test")
function path_test:test_basename()
   assert_equal("blub", path.basename("/hello/world/blub"))
end
