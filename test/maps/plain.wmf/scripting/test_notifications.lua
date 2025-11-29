include "test/scripting/stable_save.lua"

run(function()
   game.desired_speed = 1000
   sleep(2000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq = hqs[1]
   local fhq = hq.flag

   local economy_subscriber = wl.Subscriber("economy")
   assert_equal(0, economy_subscriber.size)

   prefilled_buildings(p1, { "barbarians_well", 25, 25, })
   connected_road("normal", p1, fhq, "r,r,r|", true)

   sleep(1000)
   assert_equal(4, economy_subscriber.size)
   assert_equal("merged", economy_subscriber:get(1).action)
   assert_equal("deleted", economy_subscriber:get(2).action)
   assert_equal("merged", economy_subscriber:get(3).action)
   assert_equal("deleted", economy_subscriber:get(4).action)
   economy_subscriber:clear()
   assert_equal(0, economy_subscriber.size)

   local flag = p1:place_flag(map:get_field(28, 25))
   sleep(1000)
   assert_equal(0, economy_subscriber.size)
   flag:destroy()
   sleep(1000)
   assert_equal(2, economy_subscriber.size)
   assert_equal("deleted", economy_subscriber:get(1).action)
   assert_equal("deleted", economy_subscriber:get(2).action)
   sleep(1000)

   local well = p1:get_buildings("barbarians_well")[1]

   local muted_subscriber = well:subscribe_to_muted()
   local removed_subscriber = well:subscribe_to_removed()

   assert_equal(0, muted_subscriber.size)
   assert_equal(0, removed_subscriber.size)

   sleep(1000)

   wl.ui.MapView():click(well.fields[1])
   wl.ui.MapView():get_child("mute_this"):click()
   sleep(1000)

   stable_save(game, "notifications", 1000)

   assert_equal(2, economy_subscriber.size)
   assert_equal("deleted", economy_subscriber:get(1).action)
   assert_equal("deleted", economy_subscriber:get(2).action)

   assert_equal(1, muted_subscriber.size)
   assert_equal(0, removed_subscriber.size)

   assert_error("Get out of range", function() muted_subscriber:get(0) end)
   assert_error("Get out of range", function() muted_subscriber:get(2) end)
   assert_error("Pop out of range", function() removed_subscriber:pop(1) end)

   sleep(1000)

   assert_equal(well, muted_subscriber:get(1).object)
   assert_equal(1, muted_subscriber.size)

   assert_equal(well, muted_subscriber:pop(1).object)
   assert_equal(0, muted_subscriber.size)

   sleep(1000)
   well:destroy()
   assert_equal(0, muted_subscriber.size)
   assert_equal(1, removed_subscriber.size)
   assert_equal(well.serial, removed_subscriber:get(1).object)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
