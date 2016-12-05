run(function()
   sleep(100)

   game.desired_speed = 50 * 1000

   create_southern_port()
   create_northern_port()

   -- create a ready-to-work lumberjack connected to the northern port
   local lumberjack_hut = p1:place_building("barbarians_lumberjacks_hut", map:get_field(18, 4), false, true)
   if lumberjack_hut.valid_workers then lumberjack_hut:set_workers(lumberjack_hut.valid_workers) end
   connected_road(p1, map:get_field(18,5).immovable, "l,l|tl,tr|", true)

   -- a long street to capture the woodcutter when his house is burned
   connected_road(p1, map:get_field(18,5).immovable, "r,r|r,r|r,tr|tr,tl|l,l|l,l|l,l|", true)

   -- plant a tree inside the street to the right of the lumberjack hut

   assert_equal(nil, map:get_field(23,3).immovable)
   map:place_immovable("oak_summer_old", map:get_field(23,3), "world")
   assert_not_equal(nil, map:get_field(23,3).immovable)

   -- set logs and woodcutters to "remove from here" in the port
   np = northern_port()
   np:set_ware_policies("log", "remove")
   np:set_worker_policies("barbarians_lumberjack", "remove")

   -- aaand action!

   -- sleep until the tree has been felled
   while (map:get_field(23,3).immovable.descr:has_attribute("tree")) do
      sleep(100)
   end
   -- some more sleep since "removal of tree" and "worker has log" are not at the same time
   sleep(1000)
   -- now the worker should have the tree


   lumberjack_hut:destroy()
   -- worker will find street and enter port, can't stay there, enters ship
   while not (ship:get_workers() == 1) do
      sleep(100)
   end
   assert_equal(1, ship:get_workers())
   -- no wares since the worker is still carrying the log
   assert_equal(0, ship:get_wares())

   -- start to build a lumberjack on the second port while the required log is still in transit
   local cons = p1:place_building("barbarians_lumberjacks_hut", map:get_field(17, 17), true, true)
   connected_road(p1, map:get_field(18,18).immovable, "l,tl|", true)

   -- give the log time to reach the building site
   while not (ship:get_workers() == 0) do
      sleep(100)
   end
   sleep(3000)

   cons:destroy()
   
   sleep(6000)
   -- check if both lumberjack and log come to rest in the port
   -- if it is possible to check for the log in the building site that would be an alternative
   assert_equal(1, southern_port():get_wares("log"))
   assert_equal(1, southern_port():get_workers("barbarians_lumberjack"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
