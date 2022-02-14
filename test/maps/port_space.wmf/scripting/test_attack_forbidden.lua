run(function()
   sleep(1000)

   -- Initially, no overrides exist
   assert_equal(false, p1:is_attack_forbidden(2))
   assert_equal(false, p2:is_attack_forbidden(1))
   -- No dumbness checks should take place
   assert_equal(false, p1:is_attack_forbidden(1))
   assert_equal(false, p2:is_attack_forbidden(2))
   assert_equal(false, p1:is_attack_forbidden(3))

   p1:set_attack_forbidden(2, true)

   assert_equal(true, p1:is_attack_forbidden(2))
   assert_equal(false, p2:is_attack_forbidden(1))
   assert_equal(false, p1:is_attack_forbidden(3))

   p2:set_attack_forbidden(3, true)

   assert_equal(true, p1:is_attack_forbidden(2))
   assert_equal(false, p2:is_attack_forbidden(1))
   assert_equal(false, p1:is_attack_forbidden(3))
   assert_equal(true, p2:is_attack_forbidden(3))

   p2:set_attack_forbidden(1, true)

   assert_equal(true, p1:is_attack_forbidden(2))
   assert_equal(true, p2:is_attack_forbidden(1))
   assert_equal(true, p2:is_attack_forbidden(3))

   p2:set_attack_forbidden(1, false)
   p1:set_attack_forbidden(2, false)

   assert_equal(false, p2:is_attack_forbidden(1))
   assert_equal(false, p1:is_attack_forbidden(2))
   assert_equal(true, p2:is_attack_forbidden(3))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
