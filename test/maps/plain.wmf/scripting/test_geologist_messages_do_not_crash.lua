run(function()
   sleep(5000)

   assert_equal(1, #p1:get_buildings("barbarians_headquarters"))

   local hqs = p1:get_buildings("barbarians_headquarters")
   local hq = hqs[1]
   local fhq = hq.flag
   hq:set_workers("barbarians_geologist", 2)

   local field = map:get_field(25,22)
   local flag = p1:place_flag(field)
   local road = p1:place_road(flag, "bl", "bl", "bl", "bl")
   sleep(1000)

   local mv = wl.ui.MapView()
   mv:click(field)
   sleep(1000)

   mv.windows.field_action.buttons.geologist:click()
   sleep(1000)

   sleep(360000)

   assert_true(#p1.inbox >= 6)
   mv.buttons.messages:click()

   for t=0,6 do
     sleep(1000)
     mv.windows.messages.buttons.archive_or_restore_selected_messages:click()
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
