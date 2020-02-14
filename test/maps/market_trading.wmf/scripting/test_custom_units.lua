run(function()
   sleep(2000)
   prefilled_buildings(p1, { "barbarians_custom_building", 22, 27 })
   custom_building = map:get_field(22, 27).immovable
   connected_road(p1, custom_building.flag, "tr,tl|", true)

   sleep(100000)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
