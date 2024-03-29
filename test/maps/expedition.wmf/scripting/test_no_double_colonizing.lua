run(function()
   port:set_wares {
      blackwood = 100,
      log = 100,
      gold = 20,
      granite = 20,
      grout = 20,
      iron = 20,
      reed = 20,
   }
   port:set_workers("barbarians_builder", 5)

   create_two_ships()
   port:start_expedition()
   wait_for_message("Expedition")

   local mv = wl.ui.MapView()
   local pf = port.fields[1]
   assert_nil(mv.windows.building_window)
   mv:click(pf)
   while(mv.windows.building_window == nil) do
      sleep(100)
   end
   local port_window = mv.windows.building_window
   assert_nil(port_window.tabs.expedition)

   port:start_expedition()
   while(port_window.tabs.expedition_wares_queue == nil) do
      sleep(100)
   end
   port_window.tabs.expedition_wares_queue:click()

   wait_for_message("Expedition")
   assert_nil(port_window.tabs.expedition_wares_queue)
   port_window:close()

   game.desired_speed = 2 * 1000

   first_ship.island_explore_direction="ccw"
   sleep(10000)
   assert_equal("ccw", first_ship.island_explore_direction)
   wait_for_message("Port Space")
   second_ship.island_explore_direction="ccw"
   sleep(10000)
   assert_equal("ccw", second_ship.island_explore_direction)
   wait_for_message("Port Space")

   second_ship:build_colonization_port()
   sleep(5000)
   first_ship:build_colonization_port()
   -- This test is to check that there won't be any crash now
   wait_for_message("Port")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
