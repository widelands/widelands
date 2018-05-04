-- Test writing and reading of campaign data

run(function()
   sleep(5000)

   local game = wl.Game()

   print("Writing campaign data...")
   game:save_campaign_data("test_suite_campaign_data", "test_suite_campaign_data", {
      abc = "Hello",
      d = 8,
      efg = "World",
      h = -999,
      i = true,
      jklmno = 0,
      pq = "Hello",
      rst = {
         false,
         "abc",
         -1,
         20,
         { hello = "World" }
      },
      uv = 999,
      wxyz = false
   })
   print("Done.")

   print("Reading campaign data...")
   local result = game:read_campaign_data("test_suite_campaign_data", "test_suite_campaign_data")
   assert_equal("string", type(result.abc))
   assert_equal("Hello", result.abc)

   assert_equal("number", type(result.d))
   assert_equal(8, result.d)

   assert_equal("string", type(result.efg))
   assert_equal("World", result.efg)

   assert_equal("number", type(result.h))
   assert_equal(-999, result.h)

   assert_equal("boolean", type(result.i))
   assert_equal(true, result.i)

   assert_equal("number", type(result.jklmno))
   assert_equal(0, result.jklmno)

   assert_equal("string", type(result.pq))
   assert_equal("Hello", result.pq)

   assert_equal("number", type(result.uv))
   assert_equal(999, result.uv)

   assert_equal("boolean", type(result.wxyz))
   assert_equal(false, result.wxyz)

   assert_equal("boolean", type(result.rst[1]))
   assert_equal(false, result.rst[1])

   assert_equal("string", type(result.rst[2]))
   assert_equal("abc", result.rst[2])

   assert_equal("number", type(result.rst[3]))
   assert_equal(-1, result.rst[3])

   assert_equal("number", type(result.rst[4]))
   assert_equal(20, result.rst[4])

   assert_equal("string", type(result.rst[5].hello))
   assert_equal("World", result.rst[5].hello)

   print("Done.")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
