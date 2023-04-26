run(function()
   sleep(100)

   -- Test saveloading for foreign militarysite
   prefilled_buildings(p1, { "frisians_fortress", 11, 24 })

   -- Test foreign immovables and their transformations
   map:place_immovable("berry_bush_strawberry_tiny", map:get_field(24, 28 ))
   map:place_immovable("barleyfield_tiny", map:get_field(26, 29 ))

   -- Ca. 11 minutes until we have a ripe barleyfield. Sleep half now, half after saving
   sleep(1000 * 60 * 6)

   stable_save(game, "foreign_units", 50 * 1000)

   sleep(1000 * 60 * 6)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
