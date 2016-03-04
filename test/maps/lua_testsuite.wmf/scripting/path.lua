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

function path_test:test_list_files()
   local dirname = "tribes/buildings/productionsites/barbarians/lumberjacks_hut/"
   assert_equal(4, #path.list_files(dirname .. "build_??.png"))
   assert_equal("tribes/buildings/productionsites/barbarians/lumberjacks_hut/build_00.png", path.list_files(dirname .. "build_??.png")[1])
   assert_equal("tribes/buildings/productionsites/barbarians/lumberjacks_hut/build_03.png", path.list_files(dirname .. "build_??.png")[4])
   assert_equal(1, #path.list_files(dirname .. "menu.png"))
   assert_equal("tribes/buildings/productionsites/barbarians/lumberjacks_hut/menu.png", path.list_files(dirname .. "menu.png")[1])
   assert_equal(0, #path.list_files(dirname .. "foo.png"))
   assert_equal(1, #path.list_files(dirname .. "init.lua"))
end
