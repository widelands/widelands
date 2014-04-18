-- =============================
-- Test path module
-- =============================
path_test = lunit.TestCase("path tests")
function path_test:test_basename()
   assert_equal("blub", path.basename("/hello/world/blub"))
   assert_equal("blub", path.basename("blub"))
end

function path_test:test_dirname()
   assert_equal("/hello/world/", path.dirname("/hello/world/blub"))
   assert_equal("hello/world/", path.dirname("hello/world/blub"))
   assert_equal("", path.dirname("blub"))
end
