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

   sleep(12000)

   -- remove the ship while the ware is in transit.
   stable_save("0_before_removing_ship")
   ship:remove()
   sleep(1000)
   stable_save("1_no_more_ship")

   assert_equal(1, p1:get_wares("blackwood"))
   -- It is not in the port (still in the dock)
   assert_equal(0, port1():get_wares("blackwood"))

   local new_ship = p1:place_bob("ship", map:get_field(10, 10))
   sleep(1000)

   assert_equal(1, p1:get_wares("blackwood"))

   -- Wait till the blackwood gets consumed.
   while p1:get_wares("blackwood") == 1 do
      sleep(2000)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
