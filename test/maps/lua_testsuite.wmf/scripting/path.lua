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

function _array_contains(list, string)
   for unused_i,value in ipairs(list) do
      print(("value: %q"):format(value))
      if value == string then return true end
   end
   return false
end

function path_test:test_list_directory()
   assert_true(_array_contains(path.list_directory("/"), "world"))
   assert_false(_array_contains(path.list_directory("/", "[^w].*"), "world"))
   local specific = path.list_directory("/pics", "cursor\\.png")
   assert_equal(#specific, 1)
   assert_equal(specific[1], "pics/cursor.png")
   local no_slash = path.list_directory("pics", "cursor\\.png")
   assert_equal(#no_slash, 1)
   assert_equal(no_slash[1], "pics/cursor.png")
end
