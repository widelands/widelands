run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_southern_port()
   create_northern_port()

   start_building_farm()
   southern_port():set_wares{
      blackwood = 1,
   }
   northern_port():set_workers{
      barbarians_builder = 1,
   }

   sleep(6000)

   -- remove the ship while the ware is in transit.
   stable_save(game, "0_before_removing_ship", 1000)
   ship:remove()
   sleep(1000)
   stable_save(game, "1_no_more_ship", 10 * 1000)

   assert_equal(1, p1:get_wares("blackwood"))
   -- It is not in the port (still in the dock)
   assert_equal(0, southern_port():get_wares("blackwood"))

   local new_ship = p1:place_ship(map:get_field(10, 10))
   sleep(1000)

   assert_equal(1, p1:get_wares("blackwood"))

   -- Wait till the blackwood gets consumed.
   while p1:get_wares("blackwood") == 1 do
      sleep(2000)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
