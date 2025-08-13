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

   p1:place_pinned_note(map:get_field(11, 11), "note in water") -- to be listed as ship dest

   create_two_ships()
   port:start_expedition()
   wait_for_message("Expedition")

   local mv = wl.ui.MapView()

   -- open port window
   local pf = port.fields[1]
   local pw_name = string.bformat("building_window_%u", port.serial)
   assert_nil(mv.windows.pw_name, "## port window was open ##")
   mv:click(pf)

   -- open ship windows
   local sw1_name = string.bformat("shipwindow_%u", first_ship.serial)
   assert_nil(mv.windows[sw1_name], "## ship 1 window was open ##")
   mv:click(first_ship.field)

   local sw2_name = string.bformat("shipwindow_%u", second_ship.serial)
   assert_nil(mv.windows[sw2_name], "## ship 2 window was open ##")
   mv:click(second_ship.field)

   while(mv.windows[pw_name] == nil or
         mv.windows[sw1_name] == nil or
         mv.windows[sw2_name] == nil)
   do
      sleep(100)
   end

   local port_window = mv.windows[pw_name]
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

   sleep(1000)
   assert_equal("exp_waiting", first_ship.state, "1st ship waits for instructions")
   assert_equal("transport", second_ship.state, "2nd ship goes back")

   -- check sending to a destination and then canceling the expedition does not raise an error
   if not mv.windows[sw1_name] then -- open ship window if closed
      mv:click(first_ship.field)
      while(mv.windows[sw1_name] == nil) do
         sleep(100)
      end
   end
   assert_nil(first_ship.destination, "no destination")
   mv.windows[sw1_name].dropdowns.set_destination:highlight_item(2) -- 1 is none, select after
   sleep(500)
   mv.windows[sw1_name].dropdowns.set_destination:select()
   sleep(500)
   assert_not_nil(first_ship.destination, "destination should be set")
   if mv.windows[sw2_name] then
      mv.windows[sw2_name]:close() -- to let cancel_...() take the correct ship
   end
   cancel_expedition_in_shipwindow(first_ship)
   sleep(500)
   assert_equal("transport", first_ship.state) -- expedition is canceled

   wait_for_message("Port")

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
