-- -- =======================================================================
-- --                          See Fields/Hide Fields
-- -- =======================================================================

run(function()
   function cleanup(field)
      p1:hide_fields(field:region(1))
      p1.see_all = false
   end

   sleep(5000)
   -- NOCOM local seen_field = wl.Game().map:get_field(13, 30)
   local field = wl.Game().map:get_field(10, 46)
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

   -- Unexplore. This test must go last, because we change the state of p1:seen_field.
   p1:hide_fields(field:region(1), false)
   sleep(1000)
   game.desired_speed = 0;
   assert_equal(false, p1:sees_field(field))
   assert_equal(true, p1:seen_field(field))

   p1.see_all = true
   assert_equal(true, p1:sees_field(field))
   p1.see_all = false
   assert_equal(false, p1:sees_field(field))

   p1:hide_fields(field:region(1), true)
   sleep(1000)
   game.desired_speed = 0;
   assert_equal(false, p1:sees_field(field))
   assert_equal(false, p1:seen_field(field))
   p1:reveal_fields(field:region(1))
   cleanup(field)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
