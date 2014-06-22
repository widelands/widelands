run(function()
   game.desired_speed = 1000
   sleep(1000)

   assert_equal(1, #p1:get_buildings("headquarters"))

   local hqs = p1:get_buildings("headquarters")
   local hq = hqs[1]
   local fhq = hq.flag
   hq:set_workers("geologist", 2)

   local field = map:get_field(26,24)
   local flag = p1:place_flag(field)
   local road = p1:place_road(flag, "bl", "bl", "l", "l")
   sleep(1000)

   local mv = wl.ui.MapView()
   mv:click(field)
   sleep(1000)

   mv.windows.field_action.buttons.geologist:click()
   sleep(1000)

   local field2 = map:get_field(23,23)
   local flag2 = p1:place_flag(field2)
   local road2 = p1:place_road(flag2, "br", "bl", "bl")
   sleep(1000)

   mv:click(field2)
   sleep(1000)

   mv.windows.field_action.buttons.geologist:click()
   sleep(1000)

   sleep(5555500000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
