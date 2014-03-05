-- =============================
-- Test string.bformat
-- =============================
string_bformat_test = lunit.TestCase("math.random test")
function string_bformat_test:test_simple()
   local str = ("%s %s"):bformat("Hello", "World")
   assert_equal("Hello World", str)
end
function string_bformat_test:test_reordering()
   local str = ("%2% %1%"):bformat("Hello", "World")
   assert_equal("World Hello", str)
end
function string_bformat_test:test_integer()
   assert_equal("1", ("%i"):bformat(1))
end
function string_bformat_test:test_float()
   assert_equal("1.00", ("%.2f"):bformat(1))
   assert_equal("3.14", ("%.2f"):bformat(3.14151))
end
function string_bformat_test:test_wrong_number_of_args()
   assert_error("too few", function()
      ("%s %s"):bformat(3.14151)
   end)
   assert_error("too many", function()
      ("%s %s"):bformat(3.14151, 1, 2)
   end)
end
