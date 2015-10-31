include "scripting/table.lua"

test_table = lunit.TestCase("Auxilary table tests")
function test_table:test_combine()
   a = { [1] = "Hallo", [2] =  "Welt" }
   b = { "a", "b", "c" }

   rv = array_combine(a,b)

   assert_equal(5, #rv)

   assert_equal("Hallo", rv[1])
   assert_equal("Welt", rv[2])
   assert_equal("a", rv[3])
   assert_equal("b", rv[4])
   assert_equal("c", rv[5])
end
