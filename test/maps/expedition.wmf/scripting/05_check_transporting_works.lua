run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Some optimization. No need to feed the hardener and to wait for trunks.
   p1:get_buildings("hardener")[1]:remove()
   hq:set_wares("trunk", 100)
   port:set_wares("blackwood", 100)


   start_expedition()
   wait_for_message("Expedition ready")
   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   wait_for_message("Port space found")
   assert_true(click_button("buildport"))
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))
   wait_for_message("Port")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   stable_save("port_done")
   game.desired_speed = 25 * 1000

   -- build a lumberjack and see if the ship starts transporting stuff
   p1:place_building("lumberjacks_hut", map:get_field(17, 1), true)
   connected_road(p1, map:get_field(18,2).immovable, "bl,l|", true)
   while map:get_field(17, 1).immovable.name ~= "lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("builder"))

   -- build a lumberjack and see if the builder gets transported
   p1:place_building("lumberjacks_hut", map:get_field(12, 18), true)
   while map:get_field(12, 18).immovable.name ~= "lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("builder"))

   -- Check that the first lumberjack house got his worker.
   assert_equal(1, map:get_field(17, 1).immovable:get_workers("lumberjack"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
