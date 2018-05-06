-- -- =======================================================================
-- --                          See Fields/Hide Fields
-- -- =======================================================================

run(function()
   function cleanup(field)
      p1:hide_fields(field:region(1))
      p1.see_all = false
   end

   sleep(5000)
   local field = wl.Game().map:get_field(50, 20)
   cleanup(field)

   -- Seen field
   assert_equal(false, p1:sees_field(field))
   assert_equal(false, p1:seen_field(field))
   p1:reveal_fields(field:region(1))
   sleep(1000)
   p1:hide_fields(field:region(1))
   sleep(1000)
   assert_equal(false, p1:sees_field(field))
   assert_equal(true, p1:seen_field(field))
   cleanup(field)

   -- Sees field
   assert_equal(false, p1:sees_field(field))
   p1:reveal_fields(field:region(1))
   sleep(1000)
   assert_equal(true, p1:sees_field(field))
   p1:hide_fields(field:region(1))
   sleep(1000)
   assert_equal(false, p1:sees_field(field))
   cleanup(field)

   -- See all
   assert_equal(false, p1:sees_field(field))
   p1.see_all = true
   assert_equal(true, p1.see_all)
   assert_equal(true, p1:sees_field(field))
   p1.see_all = false
   assert_equal(false, p1:sees_field(field))
   cleanup(field)

   -- Sees field see all hide
   p1.see_all = true
   assert_equal(true, p1:sees_field(field))
   p1:hide_fields(field:region(1))
   sleep(1000)
   assert_equal(true, p1:sees_field(field))
   p1.see_all = false
   assert_equal(false, p1:sees_field(field))
   cleanup(field)

   p1:reveal_fields(field:region(1))
   sleep(1000)
   assert_equal(true, p1:sees_field(field))
   p1.see_all = false
   assert_equal(true, p1:sees_field(field))
   cleanup(field)

   -- Unexplore
   p1:hide_fields(field:region(1), false)
   sleep(1000)
   assert_equal(false, p1:sees_field(field))
   assert_equal(true, p1:seen_field(field))

   p1.see_all = true
   assert_equal(true, p1:sees_field(field))
   p1.see_all = false
   assert_equal(false, p1:sees_field(field))

   p1:hide_fields(field:region(1), true)
   sleep(1000)
   assert_equal(false, p1:sees_field(field))
   assert_equal(false, p1:seen_field(field))
   p1:reveal_fields(field:region(1))
   sleep(1000)
   assert_equal(true, p1:sees_field(field))
   assert_equal(true, p1:seen_field(field))
   cleanup(field)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
