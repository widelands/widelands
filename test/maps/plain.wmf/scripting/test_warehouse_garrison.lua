run(function()
   print("Testing headquarters garrison settings")

   game.desired_speed = 1000
   sleep(1000)

   local hqs = p1:get_buildings("barbarians_headquarters")
   assert_equal(1, #hqs)
   local hq = hqs[1]
   local fhq = hq.flag

   hq:set_soldiers{
      -- Mixed order, but they should be sorted on insertion
      [{0, 2, 0, 2}] = 1,
      [{0, 3, 0, 2}] = 1,
      [{3, 5, 0, 2}] = 1,
      [{0, 0, 0, 1}] = 1,
      [{0, 4, 0, 2}] = 1,
      [{0, 0, 0, 0}] = 1,
      [{0, 5, 0, 2}] = 1,
      [{2, 5, 0, 2}] = 1,
      [{1, 5, 0, 2}] = 1,
      [{0, 1, 0, 2}] = 1,
      [{0, 0, 0, 2}] = 1,
   }

   assert_equal(11, hq:get_workers("barbarians_soldier"))

   hq.soldier_preference = "heroes"
   hq.capacity = 10  -- one free soldier

   sentry = p1:place_building("barbarians_sentry", map:get_field(24, 25), false, true)
   assert_equal("rookies", sentry.soldier_preference)
   assert_equal(2, sentry.capacity)

   -- Connect sentry to HQ, the weakest soldier should occupy the sentry
   print("Allowing only the weakest soldier to the sentry")
   sentry.soldier_preference = "heroes"
   connected_road("normal", p1, fhq, "r,r|", true)

   game.desired_speed = 10000
   sleep(15000)

   assert_equal(10, hq:get_workers("barbarians_soldier"))
   local sentry_soldiers = sentry:get_soldiers("all")
   for levels,num in pairs(sentry_soldiers) do
      assert_equal(1, num, "More than one soldiers with the same level")
      for i = 1, 4 do
         assert_equal(0, levels[i], "Wrong soldier in the sentry")
      end
   end

   -- Free up more soldiers and force exchange
   print("Allow stronger soldiers to the sentry")
   sentry.soldier_preference = "rookies"
   sentry.capacity = 1
   hq.capacity = 3
   sleep(10)
   sentry.soldier_preference = "heroes"
   sleep(20000)
   sentry.capacity = 2
   sleep(20000)

   assert_equal(9, hq:get_workers("barbarians_soldier"))
   assert_equal(0, sentry:get_soldiers{0, 0, 0, 0}, "Weakest soldier is still in the sentry")
   assert_equal(1, sentry:get_soldiers{0, 5, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(1, sentry:get_soldiers{0, 4, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(1, hq:get_soldiers{3, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{2, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{1, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(0, hq:get_soldiers{0, 5, 0, 2}, "Unexpected soldier is in the HQ")
   assert_equal(0, hq:get_soldiers{0, 4, 0, 2}, "Unexpected soldier is in the HQ")
   assert_equal(1, hq:get_soldiers{0, 3, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 0, 0, 0}, "Expected soldier is not in the HQ")

   -- Release the strongest soldiers and force exchange
   print("Allow the strongest soldiers to the sentry")
   hq.soldier_preference = "rookies"
   sleep(10)
   sentry.soldier_preference = "rookies"
   sentry.capacity = 1
   sleep(10)
   sentry.soldier_preference = "heroes"
   sleep(20000)
   sentry.capacity = 2
   sleep(20000)

   assert_equal(0, sentry:get_soldiers{0, 5, 0, 2}, "Unexpected soldier is in the sentry")
   assert_equal(0, sentry:get_soldiers{0, 4, 0, 2}, "Unexpected soldier is in the sentry")
   assert_equal(1, sentry:get_soldiers{3, 5, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(1, sentry:get_soldiers{2, 5, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(0, hq:get_soldiers{3, 5, 0, 2}, "Unexpected soldier is in the HQ")
   assert_equal(0, hq:get_soldiers{2, 5, 0, 2}, "Unexpected soldier is in the HQ")
   assert_equal(1, hq:get_soldiers{1, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 4, 0, 2}, "Expected soldier is not in the HQ")

   -- Try to get the weakest ones to go to the sentry
   print("Request rookies but don't allow the weakest soldiers to the sentry")
   sentry.soldier_preference = "rookies"
   sentry.capacity = 1
   sleep(20000)
   sentry.capacity = 2
   sleep(20000)

   assert_equal(0, sentry:get_soldiers{0, 0, 0, 0}, "Unexpected soldier is in the sentry")
   assert_equal(0, sentry:get_soldiers{3, 5, 0, 2}, "Unexpected soldier is in the sentry")
   assert_equal(1, sentry:get_soldiers{0, 1, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(1, sentry:get_soldiers{0, 2, 0, 2}, "Expected soldier is not in the sentry")
   assert_equal(1, hq:get_soldiers{3, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{2, 5, 0, 2}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 0, 0, 0}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 0, 0, 1}, "Expected soldier is not in the HQ")
   assert_equal(1, hq:get_soldiers{0, 0, 0, 2}, "Expected soldier is not in the HQ")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
