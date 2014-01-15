run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   start_building_farm()
   port1():set_wares{
      blackwood = 1,
   }
   port2():set_workers{
      builder = 1,
   }

   assert_equal(p1:get_wares("blackwood"), 1)
   while ship:get_wares() == 0 do sleep(2000) end
   assert_equal(p1:get_wares("blackwood"), 1)

   -- remove the ship while the ware is in transit.
   stable_save("0_before_removing_ship")
   ship:remove()
   sleep(1000)
   stable_save("1_no_more_ship")

   -- No more blackwood.
   assert_equal(p1:get_wares("blackwood"), 0)

   local new_ship = p1:place_bob("ship", map:get_field(10, 10))
   port1():set_wares{
      blackwood = 1,
   }
   sleep(1000)

   assert_equal(p1:get_wares("blackwood"), 1)

   -- Wait till the blackwood gets consumed.
   while p1:get_wares("blackwood") == 1 do
      sleep(2000)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
