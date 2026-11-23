include "test/scripting/stable_save.lua"

run(function()
   print("Testing trainingsite preference settings")

   game.desired_speed = 1000
   sleep(1000)

   local hqs = p1:get_buildings("barbarians_headquarters")
   assert_equal(1, #hqs)
   local hq = hqs[1]
   local fhq = hq.flag

   hq:set_soldiers{
      [{0, 0, 0, 0}] = 1,
      [{0, 0, 0, 1}] = 1,
      [{0, 0, 0, 2}] = 1,
      [{0, 1, 0, 2}] = 1,
      [{0, 2, 0, 2}] = 1,
      [{0, 3, 0, 2}] = 1,
      [{0, 4, 0, 2}] = 1,
      [{0, 5, 0, 2}] = 1,
      [{1, 5, 0, 2}] = 1,
      [{2, 5, 0, 2}] = 1,
      [{3, 5, 0, 2}] = 1,
   }

   assert_equal(11, hq:get_workers("barbarians_soldier"))

   tc = p1:place_building("barbarians_trainingcamp", map:get_field(18, 25), false, true)
   tc:set_workers("barbarians_trainer", 1)
   tc.soldier_preference = "heroes"
   tc.capacity = 3

   print("Expecting the 3 strongest soldiers go to the training camp")
   connected_road("normal", p1, fhq, "l,l|,l,l|", true)

   game.desired_speed = 50000
   while tc:get_soldiers("present") ~= 3 do
      sleep(1000)
   end
   sleep(1000)

   assert_equal(3, tc:get_soldiers("associated"), "Wrong number of soldiers in training camp")
   assert_equal(1, tc:get_soldiers{2, 5, 0, 2}, "Level 9 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{1, 5, 0, 2}, "Level 8 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 2}, "Level 7 soldier is not in the training camp")


   print("Disallow helmets, current soldiers should be kicked out")
   tc:set_desired_fill("helmet", 0)
   tc:set_desired_fill("helmet_mask", 0)
   tc:set_desired_fill("helmet_warhelm", 0)

   while tc:get_soldiers({0, 4, 0, 2}) ~= 1 do
      sleep(1000)
   end
   while tc:get_soldiers({0, 3, 0, 2}) ~= 1 do
      sleep(1000)
   end
   while tc:get_soldiers({0, 2, 0, 2}) ~= 1 do
      sleep(1000)
   end
   sleep(1000)

   assert_equal(3, tc:get_soldiers("present"), "Wrong number of soldiers in training camp")


   print("Test training strongest soldier first")
   tc:set_desired_fill("helmet", 2)
   tc:set_desired_fill("helmet_mask", 2)
   tc:set_desired_fill("helmet_warhelm", 2)
   tc:set_inputs({
      helmet = 2,
      helmet_mask = 2,
      helmet_warhelm = 2,
      fish = 6,
      barbarians_bread = 10,
   })

   while tc:get_soldiers({3, 4, 0, 2}) ~= 1 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{0, 3, 0, 2}, "Level 5 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 2, 0, 2}, "Level 4 soldier is not in the training camp")

   tc:set_inputs("fish", 6)
   tc:set_inputs("barbarians_bread", 10)
   while tc:get_soldiers({3, 3, 0, 2}) ~= 1 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{3, 4, 0, 2}, "Level 9 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 2, 0, 2}, "Level 4 soldier is not in the training camp")


   print("Test training weakest soldier first")
   tc.soldier_preference = "rookies"
   tc:set_soldiers{
      [{0, 1, 0, 0}] = 1,
      [{0, 3, 0, 1}] = 1,
      [{0, 5, 0, 2}] = 1,
   }
   tc:set_inputs({
      helmet = 2,
      helmet_mask = 2,
      helmet_warhelm = 2,
      fish = 6,
      barbarians_bread = 10,
   })

   while tc:get_soldiers({3, 1, 0, 0}) ~= 1 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{0, 3, 0, 1}, "Level 4 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 2}, "Level 7 soldier is not in the training camp")

   tc:set_inputs("fish", 6)
   tc:set_inputs("barbarians_bread", 10)
   while tc:get_soldiers({3, 3, 0, 1}) ~= 1 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{3, 1, 0, 0}, "Level 4 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 2}, "Level 7 soldier is not in the training camp")


   print("Expecting the 3 weakest soldiers go to the training camp")
   hq:set_soldiers{
      [{0, 0, 0, 0}] = 1,
      [{0, 0, 0, 1}] = 1,
      [{0, 0, 0, 2}] = 1,
      [{0, 1, 0, 2}] = 1,
      [{0, 2, 0, 2}] = 1,
      [{0, 3, 0, 2}] = 1,
      [{0, 4, 0, 2}] = 1,
      [{0, 5, 0, 2}] = 1,
      [{1, 5, 0, 2}] = 1,
      [{2, 5, 0, 2}] = 1,
      [{3, 5, 0, 2}] = 1,
   }
   tc:set_soldiers{}

   while tc:get_soldiers("present") ~= 3 do
      sleep(1000)
   end
   sleep(1000)

   assert_equal(1, tc:get_soldiers{0, 0, 0, 0}, "Level 0 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 0, 0, 1}, "Level 1 soldier is not in the training camp")
   assert_equal(1, tc:get_soldiers{0, 0, 0, 2}, "Level 2 soldier is not in the training camp")


   print("Test training with no preference")
   hq:set_wares("fish", 100)
   hq:set_wares("barbarians_bread", 100)
   tc:set_inputs{
      fish = 6,
      barbarians_bread = 10,
      ax_sharp = 2,
      ax_broad = 2,
      ax_bronze = 0,  -- force skipping a level
      ax_battle = 2,
      ax_warriors = 2,
   }
   tc.capacity = 10
   tc.soldier_preference = "any"
   tc:set_soldiers{
      [{0, 0, 0, 0}] = 2,
      [{0, 1, 0, 0}] = 2,
      [{0, 2, 0, 0}] = 2,
      [{0, 3, 0, 0}] = 2,
      [{0, 4, 0, 0}] = 2,
   }

   -- last program was health 2->3
   -- attack 2->3 is not possible, next possible is attack 3->4

   while tc:get_soldiers({0, 4, 0, 0}) ~= 3 do
      sleep(1000)
   end
   assert_equal(2, tc:get_soldiers{0, 0, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 1, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 2, 0, 0}, "Wrong number of level 2 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 3, 0, 0}, "Wrong number of level 3 soldiers in the training camp")
   assert_equal(0, tc:get_soldiers{0, 5, 0, 0}, "Wrong number of level 5 soldiers in the training camp")
   print("level 3->4 OK")

   while tc:get_soldiers({0, 5, 0, 0}) ~= 1 do
      sleep(1000)
   end
   assert_equal(2, tc:get_soldiers{0, 0, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 1, 0, 0}, "Wrong number of level 1 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 2, 0, 0}, "Wrong number of level 2 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 3, 0, 0}, "Wrong number of level 3 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 4, 0, 0}, "Wrong number of level 4 soldiers in the training camp")
   print("level 4->5 OK")

   while tc:get_soldiers({0, 1, 0, 0}) ~= 3 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{0, 0, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 2, 0, 0}, "Wrong number of level 2 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 3, 0, 0}, "Wrong number of level 3 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 4, 0, 0}, "Wrong number of level 4 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 0}, "Wrong number of level 5 soldiers in the training camp")
   print("level 0->1 OK")

   stable_save(game, "training_any", 50000)

   while tc:get_soldiers({0, 2, 0, 0}) ~= 3 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{0, 0, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 1, 0, 0}, "Wrong number of level 1 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 3, 0, 0}, "Wrong number of level 3 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 4, 0, 0}, "Wrong number of level 4 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 0}, "Wrong number of level 5 soldiers in the training camp")
   print("level 1->2 OK")

   while tc:get_soldiers({0, 4, 0, 0}) ~= 3 do
      sleep(1000)
   end
   assert_equal(1, tc:get_soldiers{0, 0, 0, 0}, "Wrong number of level 0 soldiers in the training camp")
   assert_equal(2, tc:get_soldiers{0, 1, 0, 0}, "Wrong number of level 1 soldiers in the training camp")
   assert_equal(3, tc:get_soldiers{0, 2, 0, 0}, "Wrong number of level 2 soldiers in the training camp")
   assert_equal(0, tc:get_soldiers{0, 3, 0, 0}, "Wrong number of level 3 soldiers in the training camp")
   assert_equal(1, tc:get_soldiers{0, 5, 0, 0}, "Wrong number of level 5 soldiers in the training camp")
   print("level 3->4 OK")


   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
